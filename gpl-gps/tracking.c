// tracking.c Carrier and Code tracking
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.
#include <cyg/kernel/kapi.h>

#include <stdlib.h>
#include <cyg/infra/diag.h>
#include "constants.h"
#include "tracking.h"
#include "allocate.h"
#include "gp4020.h"
#include "gp4020_io.h"
#include "message.h"

/*******************************************************************************
 * #defines
 ******************************************************************************/

// Later on, we scale 1 radian = 2^^14
#define PI_SHIFT14       (long)(0.5 + PI * (1 << 14)) // 51472
#define PI_OVER2_SHIFT14 (long)(0.5 + PI * (1 << 13)) // 25736

/*******************************************************************************
 * Global variables
 ******************************************************************************/

chan_t CH[N_CHANNELS];

unsigned short accum_status_a; // set in accum_isr
unsigned short accum_status_b; // set in accum_isr

/*******************************************************************************
 * Static (module level) variables
 ******************************************************************************/

cyg_flag_value_t nav_bits_ready;

static unsigned short CarrSrchWidth;
static unsigned short AcqThresh;

static short CarrSrchStep;
static short PullCodeK;
static short PullCodeD;
static short PullInTime;
static short PhaseTest;
static short PullCarrK;
static short PullCarrD;
static short Rms;
static short CodeCorr;
static short TrackCarrK;
static short TrackCarrD;
static short TrackCodeK;
static short TrackCodeD;
static short TrackDiv;

/*******************************************************************************
 * Write 32 bits to the 16 bit code DCO rate and carrier DCO rate registers
 ******************************************************************************/
inline void
set_code_dco_rate( unsigned long ch, unsigned long freq)
{
    volatile union _gp4020_channel_control *channel_control =
        (volatile union _gp4020_channel_control *)GP4020_CORR_CHANNEL_CONTROL;

    out16( GP4020_CORR_X_DCO_INCR_HIGH, (unsigned short)(freq >> 16));
    channel_control[ch].write.code_dco_incr_low = (unsigned short)freq;
}

inline void
set_carrier_dco_rate( unsigned short ch, unsigned long freq)
{
    volatile union _gp4020_channel_control *channel_control =
        (volatile union _gp4020_channel_control *)GP4020_CORR_CHANNEL_CONTROL;

    out16( GP4020_CORR_X_DCO_INCR_HIGH, (unsigned short)(freq >> 16));
    channel_control[ch].write.carrier_dco_incr_low = (unsigned short)freq;
}


/******************************************************************************
 * Turn a correlator channel on or off
 ******************************************************************************/
void
channel_power_control( unsigned short ch, unsigned short on)
{
    static unsigned short reset_control_shadow = 0xffff;

    if( on)
        reset_control_shadow |=  (1 << ch);
    else
        reset_control_shadow &= ~(1 << ch);

    out16( GP4020_CORR_RESET_CONTROL, reset_control_shadow);
}


/******************************************************************************
 * classic signum function written for the short datatype
 ******************************************************************************/
static inline short
sgn( short data)
// It bugs me this has to be re-invented.
// It bugs me that this could be written better.
{
    return( data < 0 ? -1: data != 0);
}


/******************************************************************************
 * Compute the approximate magnitude (square norm) of 2 numbers
 *
 * The "correct" function is naturally sqrt(a^2+b^2).
 * This computation is too slow for our application however.
 * We use the leading order approximation (mag = a+b/2) for b<a with sgn fixes.
 * This is probably as good as possible without a multiply.
 *
 * Haven't tried a fit, but based on endpoints a+(sqrt(2)-1)*b is a better
 * approximation. Everything else seems to have either a couple multiplies and
 * a divide, or an actual square root. It's a fact that if there's no hardware
 * multiply the binary square root is actually faster than a multiply on a GP
 * machine, but since the ARM7TDMI has a multiply the root is slower for us.
 ******************************************************************************/
static long
lmag( long a, long b)
{
    if( a | b)
    {
        long c, d;
        c = labs( a);
        d = labs( b);

        if( c > d)
            return( c + (d >> 1));
        else
            return( d + (c >> 1));
    }
    else return( 0);
}

// Added smag() to deal with shorts only (should speed things up)
/* Point #1, this is quite possibly _not_ faster, because the ARM internally is
 * 32bit, so each op has to be masked to 16bits. Only 16bit memory accesses for
 * data are speeded up, which this has few of.
 * Point #2, do you want to return unsigned? perhaps this doesn't matter. I'm
 * not enough of a C-head. Is this only a compile-time thing or will it force
 * some sort of run-time conversion? Signed seems safer.
 * Point #3, avoiding the library call (labs) _shouldn't_ be faster, but i
 * concede it might be, and it almost certainly isn't slower.
 */
static unsigned short
smag( short a, short b)
{
    if( a < 0) a = -a;
    if( b < 0) b = -b;

    if( a > b)
        return( a + (b >> 1));
    else
        return( b + (a >> 1));
}

/*******************************************************************************
FUNCTION fix_sqrt(long x)
RETURNS  long integer

PARAMETERS
        x long integer

PURPOSE
        This function finds the fixed point square root of a long integer

WRITTEN BY
        Clifford Kelley

*******************************************************************************/
// Same name comment as above lsqrt
// Probably a _much_ more efficient alg., but just guessing.
static long
fix_sqrt( long x)
{
    long xt,scr;
    int i;

    i=0;
    xt=x;
    do
    {
        xt=xt>>1;
        i++;
    } while( xt>0);

    i=(i>>1)+1;
    xt=x>>i;
    do
    {
        scr=xt*xt;
        scr=x-scr;
        scr=scr>>1;
        scr=scr/xt;
        xt=scr+xt;
    } while( scr!=0);

    xt=xt<<7;
    return( xt);
}


/*******************************************************************************
FUNCTION fix_atan2( long y,long x)
RETURNS  long integer

PARAMETERS
        y  long   quadrature fixed point value
        x  long   in-phase fixed point value

PURPOSE
// This is the phase discriminator function.
      This function computes the fixed point arctangent represented by
      y and x in the parameter list
      1 radian = 2^14 = 16384
      based on the power series  2^14*( (y/x)-2/9*(y/x)^3 )

// This is not a particularly good approximation.
// In particular, 0.2332025325081921, rather than 2/9 is the best
// fit parameter. The next simplest thing to do is fit {x,x^3}
// I'm assuming the interval of validity is [0,1).
// Fitting {1,x,x^3} is bad because a discriminator must come smoothly to
// zero.
// I wonder, in the average math library, if it might be faster to do the
// division un-signed?
// It's not much more expensive to fit x+a*x^2+b*x^3 (one more add).
// The compiler may not properly optimize ()/9. (I think it does.)

WRITTEN BY
    Clifford Kelley
    Fixed for y==x  added special code for x==0 suggested by Joel Barnes, UNSW
*******************************************************************************/
static inline long
fix_atan2( long y, long x)
{
    long result=0,n,n3;

    // 4 quadrants, one invalid case

    if( (x == 0) && (y == 0)) /* Invalid case */
        return( result);

    if( x > 0 && x >= labs( y))
    {
        n=(y<<14)/x;
        n3=((((n*n)>>14)*n)>>13)/9;
        result=n-n3;
    }
    else if( x <= 0 && -x >= labs(y))
    {
        n  = (y<<14)/x;
        n3 = ((((n*n)>>14)*n)>>13)/9;
        if      (y >  0) result=n-n3+PI_SHIFT14;
        else if( y <= 0) result=n-n3-PI_SHIFT14;
    }
    else if( y > 0 &&  y > labs(x))
    {
        n  = (x<<14)/y;
        n3 = ((((n*n)>>14)*n)>>13)/9;
        result = PI_OVER2_SHIFT14 - n + n3;
    }
    else if( y < 0 && -y > labs(x))
    {
        n  = (x<<14)/y;
        n3 = ((((n*n)>>14)*n)>>13)/9;
        result = -n + n3 - PI_OVER2_SHIFT14;
    }
    return( result);
}


/******************************************************************************
 * Need to set up CH[] structure and initialize the loop dynamic parameters.
 *
 * Currently called from cyg_user_start() in gpl-gps.c
 ******************************************************************************/
void
initialize_tracking( void)
{
    unsigned short ch;

    // Why are these a good choices?
    CarrSrchWidth = 40;    // search 20 frequency steps on either side
    CarrSrchStep = 4698;   // search step width = 200 Hz

    /*AcqThresh = 554;  // 35 dBHz */
    AcqThresh = 622;    /* 36 dBHz */ // Be nice to justify choices

    PullCodeK = 111;
    PullCodeD = 7;
    PullCarrK = -12;
    PullCarrD = 28;

    PullInTime = 1000;  /* 1 second */
    PhaseTest  = 500;   /* last 1/2 second of pull in, start phase test */
    Rms = 312;

    CodeCorr = 0;

    TrackCodeK = 55;
    TrackCodeD = 3;
    TrackCarrK = -9;
    TrackCarrD = 21;
    TrackDiv = 19643;

    for( ch = 0; ch < N_CHANNELS; ch++)
        if( CH[ch].state != CHANNEL_OFF)
            diag_printf("INITIALIZE TRACKING: CHANNEL NOT OFF!\n\r");

#if 0

    // Removed; trying to track this bug down. The compiler should init the
    // global structure to zero.

    /* Allocate channels */
    for( ch = 0; ch < N_CHANNELS; ch++)
        CH[ch].state = CHANNEL_OFF; // This fixes a bug, but shouldn't unless
                                   // the CH[] structure is getting overwritten
                                  // although is relying on compiler init good
                                 // in this application anyway?
#endif
}


/******************************************************************************
 FUNCTION acquire( unsigned long ch)
 RETURNS  None.

 PARAMETERS
 ch  char // Which correlator channel to use

 PURPOSE  to perform initial acquire by searching code and frequency space
 looking for a high correlation

 WRITTEN BY
 Clifford Kelley

 ******************************************************************************/
static void
acquire( unsigned long ch)
{
    volatile union _gp4020_channel_accumulators *accumulators =
        (volatile union _gp4020_channel_accumulators *)
        GP4020_CORR_CHANNEL_ACCUM;

    /* Search carrier frequency bins */
    if( abs( CH[ch].n_freq) <= CarrSrchWidth)
    {
        CH[ch].prompt_mag = smag( CH[ch].i_prompt, CH[ch].q_prompt);
        CH[ch].dither_mag = smag( CH[ch].i_dither, CH[ch].q_dither);

        // "dither" is set to LATE, this "&&" seems funny
        // I think the deal is, if this is ||, then there are a fairly large
        // number of false hits, and the search process spends a lot more time
        // in the confirm() function, which doesn't scan, so the overall search
        // rate drops and finding all the satellites takes longer.
        // Probably this illustrates a flaw in the overall search process.
        if( (CH[ch].prompt_mag > AcqThresh) && (CH[ch].dither_mag > AcqThresh))
        {
            CH[ch].state = CHANNEL_CONFIRM;
            CH[ch].i_confirm = 0;
            CH[ch].n_thresh = 0;
            return;
        }

        /* No satellite yet; try delaying the code DCO 1/2 chip */
        accumulators[ch].write.code_slew_counter = 1;
        /* WAIT 300NS UNTIL NEXT ACCESS */
        CH[ch].codephase++; // count how many code phases we've searched

        // This code was commented out... why?
        // Supposedly, we can use tracking and prompt channels in parallel.
        // Are we setting the mode right? See TRACK_SEL in the CHx_SATCNTL
        // register.
        /*
        accumulators[ch].write.code_slew_counter = 2;
        CH[ch].codephase += 2;
        */

        if( CH[ch].codephase >= 2044) // PRN code length in 1/2 chips -2
        /* I'm not convinced this number is right. 2045 sounds better
         * since slew increment is (1) */
        // All code offsets have been searched; try another frequency bin
        {
            CH[ch].codephase = 0; // reset code phase count

            /* Move to another frequency bin */
            // Note the use of carrier_corr, this is meant to be a correction
            // for estimated TCXO frequency error, currently set to zero.
            // See the comment in cold_allocate_channel()
            /* Dither around the reference frequency in linearly increasing
             * steps (Why is this better than a sweep?) */
            // Generate a search sequence: 0, 1, -1, 2, -2, ...
            // This can be re-written to avoid the multiply.
            if( CH[ch].n_freq & 1) // Odd search bins map to the "right"
            {
                CH[ch].carrier_freq = CARRIER_REF + CH[ch].carrier_corr +
                    CarrSrchStep * (1 + (CH[ch].n_freq >> 1));
            }
            else // Even search bins are to the "left" of CARRIER_REF
            {
                CH[ch].carrier_freq = CARRIER_REF + CH[ch].carrier_corr -
                    CarrSrchStep * (CH[ch].n_freq >> 1);
            }

    //CH[ch].carrier_freq = CARRIER_REF; /* DELETE ME!!! */
            /* Set carrier DCO */
            set_carrier_dco_rate( ch, CH[ch].carrier_freq);
            /* WAIT 300NS UNTIL NEXT ACCESS */

            CH[ch].n_freq++; // next time try the next search bin
        }
    }
    else
    {
        /* End of frequency search: release the channel. A mainline thread will
         * eventually allocate  another satellite PRN to this channel */
        CH[ch].state = CHANNEL_OFF;
    }
}

/*******************************************************************************
 FUNCTION confirm(unsigned long ch)
 RETURNS  None.

 PARAMETERS
 ch  char  channel number

 PURPOSE  to lock the presence of a high correlation peak using an n of m
 algorithm

 WRITTEN BY
 Clifford Kelley

*******************************************************************************/
static void
confirm( unsigned long ch)
{
    CH[ch].i_confirm++; // count number of confirm attempts

    CH[ch].prompt_mag = smag( CH[ch].i_prompt, CH[ch].q_prompt);
    CH[ch].dither_mag = smag( CH[ch].i_dither, CH[ch].q_dither);

    // In acquire, it's &&
    if( (CH[ch].prompt_mag > AcqThresh) || (CH[ch].dither_mag > AcqThresh))
        CH[ch].n_thresh++; // count number of good hits

    if( CH[ch].i_confirm > 10) // try "n" confirm attempts
    {
        if( CH[ch].n_thresh >= 8) // confirmed if good hits >= "m"
        {
            CH[ch].state = CHANNEL_PULL_IN;

            CH[ch].ch_time = 0;
            CH[ch].sum = 0;
            CH[ch].th_rms = 0;
            CH[ch].bit_sync = 0;

            CH[ch].delta_code_freq_old = 0;
            CH[ch].dcarr1 = 0;
            CH[ch].old_theta = 0;

            CH[ch].ms_sign = 0x12345; /* Some garbage data */
            CH[ch].ms_count = 0;
        }
        else
        {
            /* Keep searching - assumes search parameters are still ok */
            CH[ch].state = CHANNEL_ACQUISITION;

            /* Clear sync flags */
            CH[ch].bit_sync = 0;
        }
    }
}


/*******************************************************************************
 FUNCTION pull_in( unsigned long ch)
 RETURNS  None.

 PARAMETERS
 ch  char  channel number

 PURPOSE
 pull in the frequency by trying to track the signal with a
 combination FLL and PLL
 it will attempt to track for xxx ms, the last yyy ms of data will be
 gathered to determine if we have both code and carrier lock
 if so we will transition to track

 WRITTEN BY
 Clifford Kelley

*******************************************************************************/
static void
pull_in( unsigned long ch)
{
    long ddf, ddcar;
    long q_sum,i_sum;
    long theta,theta_dot,theta_e;
    long wdot_gain;
    long prompt_mag,dither_mag;

    // Why lmag() when it's smag() elsewhere?
    prompt_mag = lmag( CH[ch].i_prompt, CH[ch].q_prompt);
    dither_mag = lmag( CH[ch].i_dither, CH[ch].q_dither);

    CH[ch].sum += (prompt_mag + dither_mag);

    /* pull_in Code tracking loop */ // "dither" set to LATE?
    if( (prompt_mag != 0) || (dither_mag != 0))
    // This branch is probably almost always taken, so maybe skip the test?
    // If both zero the correction is not zero (2nd order) except this test
    // makes it so, which is a kink in the transfer function. Is this right?
    {
        CH[ch].delta_code_freq = PullCodeK *
            ((prompt_mag - dither_mag) << 14) / (prompt_mag + dither_mag);
        ddf = (CH[ch].delta_code_freq - CH[ch].delta_code_freq_old) * PullCodeD;

        /* Don't close the loop for 2 ms to reduce transient effects? */
        if( CH[ch].ch_time > 2)
        {
            CH[ch].code_freq += (CH[ch].delta_code_freq + ddf) >> 14;
            set_code_dco_rate( ch, CH[ch].code_freq);
        }
    }
    CH[ch].delta_code_freq_old = CH[ch].delta_code_freq;

    q_sum = CH[ch].q_dither + CH[ch].q_prompt;
    i_sum = CH[ch].i_dither + CH[ch].i_prompt;

    if( i_sum || q_sum)
        theta = fix_atan2( q_sum, -i_sum);
    else
        theta = CH[ch].old_theta;

    theta_dot = theta - CH[ch].old_theta;
    // theta_dot is a measure of frequency error, used for the FLL

    /* Increase 1 ms epoch counter modulo 20 */
    CH[ch].ms_count++;
    if( CH[ch].ms_count > 19)
        CH[ch].ms_count = 0;

    // Check if the last 20 ms have the same sign and this dump
    // is different: if so, then we just had a bit edge transition
    if( !CH[ch].bit_sync &&
        (((q_sum < 0) && (CH[ch].ms_sign == 0x00000)) ||
         ((q_sum > 0) && (CH[ch].ms_sign == 0xfffff))))
    {
        // Test if last two sums were within 1/4 radian of pi/2
        // Why exactly are we testing for this here?
        if( (labs( labs( theta) - PI_OVER2_SHIFT14) < 4096) &&
              (labs( labs( CH[ch].old_theta) - PI_OVER2_SHIFT14) < 4096))
        {
            // Let the world know we're synced to the satellite message bits
            CH[ch].bit_sync = 1;
            // sync the ms count to the bit stream
            CH[ch].ms_count = 0;
            // set the flag that tells tracking() to set the 1ms epoch counter
            // after the accumulator registers are read: this will sync the
            // epoch counter with the bit stream (and the ms_count, too).
            CH[ch].load_1ms_epoch_count = 1;
        }
    }

    /* Shift sign buffer to left */
    CH[ch].ms_sign = ((CH[ch].ms_sign << 1) & 0x000fffff);

    if( q_sum < 0)
        CH[ch].ms_sign |= 1; /* Set the LSB bit if negative */

    CH[ch].old_theta = theta;

    if( theta > 0) // in lock, theta is near (+/-)pi/2
                  // this pushes theta_e toward zero
                 // Shouldn't we track near zero instead?
        theta_e = theta - PI_OVER2_SHIFT14;
    else
        theta_e = theta + PI_OVER2_SHIFT14;
    // theta_e is the discriminated phase error for the PLL

    // Near the end of pull in, start the phase test
    if( CH[ch].ch_time > (PullInTime - PhaseTest))
        CH[ch].th_rms += (theta_e * theta_e) >> 14;

    /* pull_in Carrier tracking loop */
    if( labs( theta_dot) < 32768) // less than 0.5 radian / ms
    {
        if( q_sum | i_sum) // This check was also done above, redundant?
        {
            wdot_gain = CH[ch].ch_time / 499;
            wdot_gain *= wdot_gain;
            wdot_gain *= wdot_gain; // Net: (ch_time/499)**4
            // As this factor increases the FLL is increasingly ignored
            // in favor of the PLL.
            // (This seems an expensive technique.)

            CH[ch].dcarr =
                PullCarrK * (theta_dot * 5 / (1 + wdot_gain) + theta_e);
            ddcar = PullCarrD * (CH[ch].dcarr - CH[ch].dcarr1);

            if( CH[ch].ch_time > 5) /* Don't close the loop for 5 ms */
            {
                CH[ch].carrier_freq += (CH[ch].dcarr+ddcar) >> 14;
                set_carrier_dco_rate( ch, CH[ch].carrier_freq);
            }
        }
    }

    CH[ch].dcarr1 = CH[ch].dcarr;
    CH[ch].ch_time++;

    /* Done with pull in. Wait until the end of a data bit. */
    // Are we sure we even think we're data-locked now?
    if( (CH[ch].ms_count == 19) && (CH[ch].ch_time >= PullInTime))
    /* A bit transition will happen at the next dump. */
    {
        // This is an odd average since PullInTime is a constant
        CH[ch].avg = CH[ch].sum / PullInTime / 2;
        // Since PhaseTest is a constant, the run-time division could be avoided
        // This probably can't be done now since PhaseTest is stored in a
        // variable.
        CH[ch].th_rms = fix_sqrt( CH[ch].th_rms / PhaseTest);
        // The above two calculations appear to be unnecessary for the if
        // statement on the next line.

        if( (CH[ch].avg > (14 * Rms / 10)) &&
              (CH[ch].th_rms < 12000) && CH[ch].bit_sync)
        /* Bit edge was detected. */
        {
            /* Sufficient signal, transition to tracking mode */
            CH[ch].avg *= 20;

            CH[ch].check_average = 0;
            CH[ch].sum = 0;
            CH[ch].i_prompt_20 = 0;
            CH[ch].q_prompt_20 = 0;
            CH[ch].i_dither_20 = 0;
            CH[ch].q_dither_20 = 0;
            CH[ch].delta_code_freq = 0;

            // Tell the allocate thread that we're locking
            channel_locked( ch);

            // Officially switch modes
            CH[ch].state = CHANNEL_LOCK;
        }
        else
        {
            // We lost the pullin. Eventually, do a nice transition back to
            // confirm and/or acquire. For now, to be paranoid, just kill
            // the channel.
            CH[ch].state = CHANNEL_OFF;

#if 0       // Code that was here:
            CH[ch].state = CHANNEL_ACQUISITION;
            clear_message( ch);   // flag the message_thread that the
                                        // past subframes are no longer valid
            CH[ch].codes = 0;
            CH[ch].code_freq = CODE_REF + CodeCorr;
            set_code_dco_rate( ch, CH[ch].code_freq);

            /* Clear sync flags */
            CH[ch].bit_sync = 0;
#endif
        }
    }
}

/*******************************************************************************
 FUNCTION lock( unsigned long ch)
 RETURNS  None.

 PARAMETERS  char ch  , channel number

 PURPOSE track carrier and code, and partially decode the navigation message
 (to determine TOW, subframe etc.)

 WRITTEN BY
 Clifford Kelley
 added Carrier Aiding as suggested by Jenna Cheng, UCR
*******************************************************************************/
static void
lock( unsigned long ch)
{
    long ddf, ddcar;    //
    long q_sum, i_sum;  //

    //    unsigned short BackToPullIn = 0;

    /* 50 Hz (20ms) tracking loop */
    CH[ch].ms_count++;
    if( CH[ch].ms_count > 19) CH[ch].ms_count = 0; // Efficient modulo 20

    CH[ch].q_dither_20 += CH[ch].q_dither;
    CH[ch].q_prompt_20 += CH[ch].q_prompt;
    CH[ch].i_dither_20 += CH[ch].i_dither;
    CH[ch].i_prompt_20 += CH[ch].i_prompt;

    /* Carrier loop */
    q_sum = CH[ch].q_dither + CH[ch].q_prompt;
    i_sum = CH[ch].i_dither + CH[ch].i_prompt;

    if( (q_sum != 0) || (i_sum != 0))
    { // osgps fscanf's for the tracking gains, see GPSRCVR.CPP
        // TrackCarrK is currently set to -9 in main.c and never changed.
        // TrackCarrK is only used here
        CH[ch].dcarr =
            TrackCarrK * sgn( q_sum) * (i_sum << 14) / lmag( q_sum, i_sum);
        // == Gain * Sgn[q] * Cos[Phi]

        // TrackCarrD is currently set to 21 in main.c and never changed.
        // TrackCarrD is only used here
        ddcar = TrackCarrD * (CH[ch].dcarr - CH[ch].dcarr1);
        // == Gain * delta_ddcar

        CH[ch].carrier_freq += (CH[ch].dcarr + ddcar) >> 14; // feedback
        set_carrier_dco_rate( ch, CH[ch].carrier_freq);

        // the carrier frequency and the code frequency should be in
        // concert (+/-)... change it here, update it later
    }


    CH[ch].dcarr1 = CH[ch].dcarr;

    if( CH[ch].ms_count == 19)
    {
        // Work on sum of last 20ms of data
        CH[ch].prompt_mag = lmag( CH[ch].i_prompt_20, CH[ch].q_prompt_20);
        CH[ch].dither_mag = lmag( CH[ch].i_dither_20, CH[ch].q_dither_20);
        CH[ch].sum += CH[ch].prompt_mag + CH[ch].dither_mag;

        /* Code tracking loop */
        if( CH[ch].prompt_mag | CH[ch].dither_mag)
        {
            /* Without carrier aiding. */
            // Like before, TrackCodeK fixed @ 55 in main.c
            CH[ch].delta_code_freq =
                TrackCodeK * (CH[ch].prompt_mag - CH[ch].dither_mag);
            // Like before, TrackCodeD fixed @ 3 in main.c
            ddf = TrackCodeD * (CH[ch].delta_code_freq -
                    CH[ch].delta_code_freq_old);
            // Like before, TrackDiv fixed @ 19643 in main.c
            CH[ch].code_freq += (CH[ch].delta_code_freq + ddf) / TrackDiv;

            /* With carrier aiding. */
            /*
            ddf = (TrackCodeK*(
            (CH[ch].prompt_mag - CH[ch].dither_mag) /
            (CH[ch].prompt_mag + CH[ch].dither_mag)
                   ))<<11;

            CH[ch].delta_code_freq += ddf;

            CH[ch].code_freq = (ddf + CH[ch].delta_code_freq/TrackCodeD)>>8
            + (CARRIER_REF - CH[ch].carrier_freq)/CC_SCALE + CODE_REF;
            */

            set_code_dco_rate( ch, CH[ch].code_freq);
        }

        CH[ch].delta_code_freq_old = CH[ch].delta_code_freq;

        /* Data bit */
        CH[ch].bit = ((CH[ch].q_prompt_20 + CH[ch].q_dither_20) > 0);

        // Flag that this bit is ready to process (written to the message_flag
        // in the tracking() function after we've gone through all the channels
        nav_bits_ready |= (1 << ch);

        // Increment the time, in bits, since the week began. Used in
        // the measurement thread. Also set to the true time of
        // week when we get the TOW from a valid subframe in the
        // messages thread.
        CH[ch].time_in_bits++;
        if( CH[ch].time_in_bits >= BITS_IN_WEEK)
            CH[ch].time_in_bits -= BITS_IN_WEEK;

        // Check the satellite signal strength every 100ms
        CH[ch].check_average++;
        if( CH[ch].check_average > 4)
        {
            CH[ch].check_average = 0;

            CH[ch].avg = CH[ch].sum / 10; // FIXME stop computing this on the
                                         // fly. Pre-multiply the test threshold
                                        // Also, do this whole thing better.
            CH[ch].sum = 0;

            if( (CH[ch].bit_sync) &&
                (CH[ch].avg < SIGNAL_LOSS_AVG)) /* 33 dBHz */
            {
                /* Signal loss. Clear channel. */
                CH[ch].state = CHANNEL_OFF;
                // BackToPullIn = 1;
            }
        }

        /* Clear coherent accumulations */
        CH[ch].i_dither_20 = 0;
        CH[ch].i_prompt_20 = 0;
        CH[ch].q_dither_20 = 0;
        CH[ch].q_prompt_20 = 0;
    }

#if 0
    // eventually we should switch back to pullin if we lose lock;
    // for now we're just going to kill the channel.
    // NOTE: This is dead with the BackToPullIn above commented out.
    if( BackToPullIn == TRUE) /* Tracking failed, Go back to pull in. */
    {
        CH[ch].state = CHANNEL_PULL_IN;
        clear_message( ch);
        CH[ch].ch_time = 0;
        CH[ch].sum = 0;
        CH[ch].th_rms = 0;
        CH[ch].bit_sync = 0;

        CH[ch].delta_code_freq_old = 0;
        CH[ch].dcarr1 = 0;
        CH[ch].old_theta = 0;

        CH[ch].ms_sign = 0x54321; /* Some garbage data */
        CH[ch].ms_count = 0;
    }
#endif
}


/*******************************************************************************
 FUNCTION tracking( void)
 RETURNS  None.

 PARAMETERS  None

 PURPOSE Main routine which runs on an accum_int.

 WRITTEN BY
 Clifford Kelley
 added Carrier Aiding as suggested by Jenna Cheng, UCR
*******************************************************************************/
void
tracking( void)
{
    unsigned short      ch;
    cyg_flag_value_t    channels_off;

    volatile union _gp4020_channel_accumulators *accumulators =
        (volatile union _gp4020_channel_accumulators *)GP4020_CORR_CHANNEL_ACCUM;
    volatile union _gp4020_channel_control *channel_control =
        (volatile union _gp4020_channel_control *)GP4020_CORR_CHANNEL_CONTROL;


    // "accum_status_a" and "accum_status_b" are set in the accum_isr() when
    // clearing the interrupt

    // Sequentially check each channel for new data. Do this FAST so that
    // we get the data before we have an overrun. Note that reading
    // GP3020_CORR_ACCUM_STATUS_A also clears the ACCUM_INT interrupt.
    for( ch = 0; ch < N_CHANNELS; ch++)
    {
        // Check the bits in the accumulator status register (which was read in
        // the accum_isr() function) - DON'T READ IT AGAIN HERE! - to see if
        // the channel has dumped and is thus ready to be read.

        if( accum_status_a & (1 << ch))
        {
            /* Collect channel data accumulation. */
            CH[ch].i_dither = accumulators[ch].read.i_track;
            CH[ch].q_dither = accumulators[ch].read.q_track;
            CH[ch].i_prompt = accumulators[ch].read.i_prompt;
            CH[ch].q_prompt = accumulators[ch].read.q_prompt;

            // If the last dump was the first dump in a new satellite
            // message data bit, then lock() sets the load_1ms_epoch_flag
            // so that we can set the 1m epoch counter here. Why here?
            // GP4020 Baseband Processor Design Manual, pg 60: "Ideally,
            // epoch counter accesses should occur following the reading of
            // the accumulation register at each DUMP." Great, thanks for
            // the tip, now how 'bout you tell us WHY?!
            if( CH[ch].load_1ms_epoch_count)
            {
                /* Load 1 ms epoch counter */
                channel_control[ch].write.epoch_count_load = 1;
                /* WAIT 300NS UNTIL NEXT ACCESS */

                CH[ch].load_1ms_epoch_count = 0;
            }

            // We expect the 1ms epoch counter to always stay sync'd until
            // we lose lock. To sync the 20ms epoch counter (the upper bits)
            // we wait until we get a signal from the message thread that
            // we just got the TLM+HOW words; this means we're 60 bits into
            // the message. Since the damn epoch counter counts to *50* (?!)
            // we mod it with 60 which gives us 10 (0xA00 when shifted 8).
            // --What???
            if( CH[ch].sync_20ms_epoch_count)
            {
                channel_control[ch].write.epoch_count_load =
                    (channel_control[ch].read.epoch_check & 0x1f) | 0xa00;
                /* WAIT 300NS UNTIL NEXT ACCESS */
                CH[ch].sync_20ms_epoch_count = 0;
            }

            if( accum_status_b & (1 << ch)) // Check for missed DUMP
            {
                CH[ch].missed++;
                //Explicit reset of ACCUM status reg.s
                accumulators[ch].write.accum_reset = 0;
            }
        }
    }

    // Clear the IPC shadows (see below).
    nav_bits_ready = 0;
    channels_off = 0;

    // Finally, in a second (slower) loop, track each channel that dumped. Note
    // that channels which are CHANNEL_OFF will be allocated satellites to
    // track in a mainline thread.
    for( ch = 0; ch < N_CHANNELS; ch++)
    {
        if( (accum_status_a & (1 << ch)) && (CH[ch].state != CHANNEL_OFF))
        // We already checked for dumped channels above, can  we somehow
        // avoid checking this again??
        {
            switch( CH[ch].state)
            {
                case CHANNEL_ACQUISITION:
                    acquire( ch);
                    break;
                case CHANNEL_CONFIRM:
                    confirm( ch);
                    break;
                case CHANNEL_PULL_IN:
                    pull_in( ch);
                    break;
                case CHANNEL_LOCK:
                    lock( ch);
                    break;
                default:
                    CH[ch].state = CHANNEL_OFF;
                    // TODO: assert an error here
                    break;
            }
        }
        // If the channel is off, set a flag saying so
        if( CH[ch].state == CHANNEL_OFF)
            channels_off |= (1 << ch);

    }

    // If any channels are off, signal the allocation thread that it should
    // try to use them.
    if( channels_off)
        cyg_flag_setbits( &allocate_flag, channels_off);

    // Signal the message_thread that there are new nav-bits available.
    // Besides convenience, calling setbits here only once instead of per-bit
    // is a speed optimization.
    if( nav_bits_ready)
        cyg_flag_setbits( &message_flag, nav_bits_ready);
}
