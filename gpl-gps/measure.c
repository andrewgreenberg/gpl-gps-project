// measure.c Take measurements each TIC interrupt (~100ms)
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>
#include "measure.h"
#include "constants.h"
#include "gp4020.h"
#include "gp4020_io.h"
#include "message.h"
#include "pseudorange.h"
#include "time.h"
#include "tracking.h"

/******************************************************************************
 * Globals
 ******************************************************************************/

measurement_t   meas[N_CHANNELS];
gpstime_t       meas_time;

cyg_sem_t       measure_semaphore;


/******************************************************************************
 * Grab the time in bits from the tracking loops.
 *
 * Note, when the TIC comes right at the end of a message bit transition, then
 * an accum_int may increment a time_in_bits for one of the channels *while*
 * we're running in the measure_thread(). we handle that case by calling this
 * small routine directly from the accumulate DSR in interrupts.c .
 ******************************************************************************/
void
grab_bit_times( void)
{
    unsigned short ch;

    for( ch = 0; ch < N_CHANNELS; ch++)
        meas[ch].meas_bit_time = CH[ch].time_in_bits; // meas_bit_time
}

/******************************************************************************
 * Grab the latched measurement data from the accumulators after a TIC.
 ******************************************************************************/
void
measure_thread( CYG_ADDRWORD data) // input 'data' not used
{
    static unsigned short prev_channels;

    cyg_flag_value_t channels_ready;
    unsigned short   ch;
    unsigned short   carrier_low;
    unsigned short   carrier_high;
    unsigned short   meas_status_a;
    unsigned short   posts_left;
    unsigned short   raw_epoch;

    volatile union _gp4020_channel_control *channel_control =
            (volatile union _gp4020_channel_control *)
            GP4020_CORR_CHANNEL_CONTROL;

    cyg_semaphore_init( &measure_semaphore, 0);

    while(1)
    {
        cyg_semaphore_wait( &measure_semaphore); // Wait for acum_dsr()

        setbits32( GPS4020_GPIO_WRITE, LED2); // DEBUG

        meas_status_a = in16( GP4020_CORR_MEAS_STATUS_A);

        // We just had a Tic (~100ms) so update the receiver clock
        increment_time_with_tic();

        // Get the current GPS time to mark the time of measurement
        meas_time = get_time(); // Is the syncronization here correct?

        // If the channel is 1) locked and 2) the signal is good and 3) the
        // clock is set vaguely correctly and 4) we've set the channel's
        // time bits, THEN grab the measurement data.
        channels_ready = 0;
        for( ch = 0; ch < N_CHANNELS; ch++)
        {
            if( (CH[ch].state == CHANNEL_LOCK)
                 && CH[ch].avg > SIGNAL_LOSS_AVG
                 && (get_clock_state() >= SF1_CLOCK)
                 && messages[ch].set_ch_time)
            {
                // Grab the latched data from the correlators (in their
                // numerical order -- go optimized compiler, go!)
                meas[ch].code_phase = channel_control[ch].read.code_phase;
                carrier_low = channel_control[ch].read.carrier_cycle_low;
                meas[ch].carrier_dco_phase =
                        channel_control[ch].read.carrier_dco_phase;
                raw_epoch = channel_control[ch].read.epoch;
                meas[ch].code_dco_phase =
                        channel_control[ch].read.code_dco_phase;
                carrier_high = channel_control[ch].read.carrier_cycle_high;

                // NOTE: Carrier phase measurements are not supported (yet)
                meas[ch].carrier_cycles = (carrier_high << 16) | carrier_low;

                // If a TIC hits right before a dump, it's possible for the
                // code phase to latch 2046. In this case the epoch counter
                // won't be incremented yet. If so, increment the epoch manually
                // (see the GP4020 design manual section 7.6.17). Of course, we
                // have to handle rollover of the epoch counter too.
                if( meas[ch].code_phase < MAX_CODE_PHASE)
                {
                    // normal case
                    meas[ch].epoch_20ms = raw_epoch >> 8;
                    meas[ch].epoch_1ms = raw_epoch & 0x1f;
                }
                else // overflow
                {
                    meas[ch].code_phase -= MAX_CODE_PHASE;
                    if( (raw_epoch | 0x1f) == 19)
                    {
                        meas[ch].epoch_1ms = 0;
                        meas[ch].epoch_20ms = (raw_epoch >> 8) + 1;
                        if( meas[ch].epoch_20ms > 49) // Not sure we
                            meas[ch].epoch_20ms = 0; // need to do this?
                    }
                    else
                    {
                        meas[ch].epoch_20ms  = raw_epoch >> 8;
                        meas[ch].epoch_1ms = (raw_epoch & 0x1f) + 1;
                    }
                }

                // Note: meas_bit_time set in grab_bit_times() above.
                meas[ch].doppler = CH[ch].carrier_freq;
                meas[ch].prn = CH[ch].prn;
                meas[ch].valid = 1;

                // Tell the pseudorange thread which measurements it can use.
                // Note that we don't want to just call the pr thread from here
                // because we're in a DSR and should get out ASAP
                channels_ready |= (1 << ch);
            }
            else
            {
                // Read the code_phase counter to indicate that
                // we didn't miss any interrupts (but toss the value)
                carrier_low = channel_control[ch].read.code_phase;
                meas[ch].valid = 0; /* Can't use this measurement */
            }

            // Check to see if we missed this channel on an earlier TIC
            if( meas_status_a & (1 << ch))
                meas[ch].missed++;
        }

        // Finally, flag all valid measurements for the pseudorange thread.
        // If there are none, but there were last TIC, then explicitly clear
        // the pseudoranges. -- Is this necessary?
        // Is it really true that .._sebits(0) doesn't work?
        if( channels_ready)
            cyg_flag_setbits( &pseudorange_flag, channels_ready);
        else if( prev_channels)
            clear_pseudoranges();

        prev_channels = channels_ready;

        // Finally, make sure we didn't miss any other posts to the measure
        // semaphore- if we did, complain about it.
        posts_left = cyg_semaphore_trywait( &measure_semaphore);
        if( posts_left)
        {
            diag_printf( "MEASURE THREAD: MISSED SEMAPHORE");
            do{
                posts_left = cyg_semaphore_trywait( &measure_semaphore);
            } while( posts_left);
        }
        clearbits32( GPS4020_GPIO_WRITE, LED2); // DEBUG
    }
}
