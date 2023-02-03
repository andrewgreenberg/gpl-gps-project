// pseudorange.c Process measurements into pseudoranges
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>
#include <math.h>
#include "constants.h"
#include "pseudorange.h"
#include "gp4020.h"
#include "gp4020_io.h"
#include "measure.h"
#include "position.h"
#include "time.h"

/******************************************************************************
 * Globals
 ******************************************************************************/

cyg_flag_t    pseudorange_flag;

pseudorange_t pr[N_CHANNELS];
gpstime_t     pr_time;

pseudorange_t pr2[N_CHANNELS];
gpstime_t     pr2_time;

/******************************************************************************
 * Clear the pseudoranges (fast).
 ******************************************************************************/
void
clear_pseudoranges( void)
{
    unsigned int ch;

    // Clear the valid flags
    for( ch = 0; ch < N_CHANNELS; ch++)
    {
        pr[ch].valid = 0;  // need to clear this flag, why do anything else?
        pr[ch].prn   = 0;
        pr[ch].range = 0;
    }
    // If there are no pseudoranges, we can't position, so clear the
    // position as well
    clear_position(); // Still dislike this technique.
                      // Does it at least optimize properly?
}


/******************************************************************************
 * Calculate them pseudoranges.
 ******************************************************************************/
void
calculate_pseudorange( unsigned short ch)
{
    unsigned short bit_count_remainder;
    unsigned long  bit_count_modded;

    // The epoch counter is more accurate than our bit counter in tracking.c:
    // lock(), so make sure they agree. This means fixing up the least few
    // significant bits. We've seen them disagree by two, but no more.

    // Figure out how many "50 bit counts" are in the current bit counter.
    // Why didn't they make the counter rational, like 0 - 30 bits? Oh well.
    // TODO, re-write this to avoid the mod?
    bit_count_remainder = meas[ch].meas_bit_time % 50;
    bit_count_modded = meas[ch].meas_bit_time - bit_count_remainder;

    // Fix up in case the epoch counter is on a different side of
    // a 50 bit "word" from us. Otherwise trust the almighty epoch counter.
#define TOL 24 // 50/2 -1 // 5 is the largest seen in practice?
    if( (bit_count_remainder < TOL) && (meas[ch].epoch_20ms > (50 - TOL)))
        bit_count_modded -= 50;
    if( (bit_count_remainder > (50 - TOL)) && (meas[ch].epoch_20ms < TOL))
        bit_count_modded += 50;

    // This can all be re-written to fixed point, don't know if we should.
    // FIXME, make this efficient, clear, and concise.
    pr[ch].sat_time = .02 * (bit_count_modded + meas[ch].epoch_20ms) +
        CODE_TIME *
        (    meas[ch].epoch_1ms +
                 (1/(double)MAX_CODE_PHASE) *
                 (    meas[ch].code_phase +
                      (1/(double)(CODE_DCO_LENGTH)) *
                          meas[ch].code_dco_phase
                 )
        );
    //        - QUARTER_CHIP_TIME; // Why this? What about analog delay?

    // NO ionospheric corrections, no nuthin': mostly for debug
    // FIXME: What about GPS week for this calculation?!
    pr[ch].range = (pr_time.seconds - pr[ch].sat_time) * SPEED_OF_LIGHT;

    // Record the following information for debugging
    pr[ch].prn = meas[ch].prn;
    pr[ch].bit_time =  bit_count_modded;
    pr[ch].epoch_20ms = meas[ch].epoch_20ms;
    pr[ch].epoch_1ms = meas[ch].epoch_1ms;
    pr[ch].code_phase = meas[ch].code_phase;
    pr[ch].code_dco_phase = meas[ch].code_dco_phase;
}

/******************************************************************************
 * Wake up on valid measurements and produce pseudoranges. Flag the navigation
 * thread if we have four or more valid pseudoranges
 ******************************************************************************/
void
pseudorange_thread( CYG_ADDRWORD data) // input 'data' not used
{
    cyg_flag_value_t measurements_ready;
    unsigned short   ch;
    unsigned short   pr_count;

    // There's no way that we're going to get a bit before this thread
    // is first executed, so it's ok to run the flag init here.
    cyg_flag_init( &pseudorange_flag);

    while(1)
    {
        // Sleep here until the measurement thread flags at least one valid
        // pseudorange measurement.
        measurements_ready = cyg_flag_wait( &pseudorange_flag,
                             (1 << N_CHANNELS) - 1,  // 0xfff (all channels)
                             CYG_FLAG_WAITMODE_OR |  // any flag
                             CYG_FLAG_WAITMODE_CLR); // clear flags

        setbits32( GPS4020_GPIO_WRITE, LED6);   // DEBUG

        // Copy over the measurement time so we know when the rho's were
        // computed.
        pr_time = meas_time;

        // OK we're awake: for each measurement that we get, (Which we assume
        // is valid, since it wouldn't get up to this thread if it weren't!),
        // produce a pseudorange. Clear it to zero if it's not valid.
        pr_count = 0;
        for( ch = 0; ch < N_CHANNELS; ++ch)
        {
            if( measurements_ready & (1 << ch))
            {
                    calculate_pseudorange( ch);
                    pr[ch].valid = 1;
                    pr_count++;
            }
            else
                pr[ch].valid = 0;
        }

        // If we have any satellites, send them to the position thread. But
        // note that the position thread is going to take a BZILLION years to
        // process it all. Because we want to keep producing psuedoranges while
        // the position thread runs, we copy over the pr's so the position
        // thread can use a private copy.
        if( pr_count > 0)
        {
#ifdef ENABLE_PSEUDORANGE_LOG
            // If it's enabled, log the pseudorange data
            log_pseudoranges();

#else       // If we log pseudoranges, don't do position fixes???
            if( !pr2_data_fresh)
            {
                for( ch = 0; ch < N_CHANNELS; ++ch)
                    pr2[ch] = pr[ch]; // Better to avoid this copy (later!)
                pr2_time = pr_time;

                // FIXME pr2_data_fresh should be IPC; what's best? nothing
                // seems to fit this case!
                pr2_data_fresh = 1; // Indicate new data in pr2

                // Signal the position thread which will reset pr2_data_fresh
                // when it's done.
                cyg_semaphore_post( &position_semaphore);
            }
#endif
        }
        else
        {
            // Uh... no pseudoranges? That's bad.
            diag_printf( "PSEUDORANGE: CALLED WITH NO SATELLITES.\n");
            clear_pseudoranges();
        }
        clearbits32( GPS4020_GPIO_WRITE, LED6); // DEBUG
    }
}
