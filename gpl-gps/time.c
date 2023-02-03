// time.c Time functions
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#include <cyg/infra/diag.h>
#include "constants.h"
#include "time.h"

static gpstime_t       time;         // receiver local time
static clock_state_t   clock_state;


/******************************************************************************
 * Increment the time
 ******************************************************************************/
void
increment_time_with_tic( void)
{
    time.seconds += TIC_PERIOD;

    if( time.seconds >= SECONDS_IN_WEEK)
    {
        time.seconds -= SECONDS_IN_WEEK;
        time.weeks++;
    }

    // and JUST in case it's the beginning of week and the clock
    // correction is > 100ms...
    else if( time.seconds < 0)
    {
        time.seconds += SECONDS_IN_WEEK;
        time.weeks--;
    }
}


/******************************************************************************
 * Set the receiver's time given the TOW from the HOW. (If it hasn't been done
 * already.) Of course we don't know what the week number is.
 ******************************************************************************/
void
set_time_with_tow( unsigned long tow)
{
    if( clock_state == NO_CLOCK)
    // This is an effort to set time.seconds to a coarse value exactly once.
    // Each time a subframe with valid TLM/HOW is found, this is called
    // (maximum average twice per second). We rely on startup initialization of
    // a static variable (time.seconds) to zero. It's not clear what happens if
    // this initialization is not done, or not re-done if required. It seems
    // both inefficient and oddly mode-ful. Possibly exception handling, or a
    // mainline or navigation handler would be better.
    {

        // TOW from the HOW is actually the start time of the next subframe so
        // deal with that, and of course we don't want negative time. The 0.07
        // sec. is the typical satellite-to-receiver signal propagation time.
        // TOW is in units of 6 seconds (length of a subframe), and we get the
        // TOW only at the 60th bit of the 300 bit, 50bps, subframe.
        if( tow)
            time.seconds = 0.07 + (double)tow * 6.0 - 240.0 * 0.020;
        else
            time.seconds = 0.07 + SECONDS_IN_WEEK   - 240.0 * 0.020;

        // We now have a clock from the HOW
        clock_state = HOW_CLOCK;
    }
}


/******************************************************************************
 * Get the clock state
 ******************************************************************************/
void
set_clock_correction( double correction)
{
    if( clock_state == FIX_CLOCK)
    {
        // Use the clock correction from the position solution and flag that we
        // have it. BUT ONLY if the correction is reasonable... _but_
        // it should be reasonble in the first place, so...
        // FIXME, this is a total hack
        if( correction > 1.0E-5)
            time.seconds -= 1.0E-5;

        else if( correction < -1.0E-5)
            time.seconds -= -1.0E-5;

        else
        {
            time.seconds -= correction;
        }
    }

    else if( clock_state == SF1_CLOCK)
    {
        // This is the first fix. So accept big changes, up to the maximum
        // possible satellite signal propagation time. With an orbital height
        // of 24,000km, we can round up to 30,000 km and get a nice round 100ms
        if( (correction < -100E-3) || (correction > 100E3))
            diag_printf("TIME.C: INITIAL CLOCK CORRECTION TOO BIG!");
        else
        {
            time.seconds -= correction;
            clock_state = FIX_CLOCK;
        }
    }
}


/******************************************************************************
 * Get the current receiver time.
 ******************************************************************************/
gpstime_t
get_time( void)
{
    return( time);
}


/******************************************************************************
 * Get the current receiver time, in plain seconds.
 ******************************************************************************/
double
get_time_in_seconds( void)
{
    return( time.seconds);
}


/******************************************************************************
 * Set the receiver's week only if we haven't before.
 ******************************************************************************/
void
set_time_with_weeks( unsigned short weeks)
{
    if( clock_state == HOW_CLOCK)
    {
        // Note that we have NO stinking clue what the true year is because the
        // week is modulo 1024 which is about 20 years. So we'll just guess
        // it's past 2000 :) and before ~ 2020 which means adding 1024 to the
        // current week. TODO: Check to see a local compile time flag, and
        // if the week number is less, then roll over once more?
        time.weeks = weeks + 1024;
        clock_state = SF1_CLOCK;
    }
}


/******************************************************************************
 * Change the clock state to fix, called from position.c
 ******************************************************************************/
void
set_to_fix_clock( void)
{
    clock_state = FIX_CLOCK;
}


/******************************************************************************
 * Get the clock state
 ******************************************************************************/
clock_state_t
get_clock_state( void)
{
    return( clock_state);
}


/******************************************************************************
 * Translate from GPS time (weeks/seconds) to normal time yyy/mm/dd hh:mm
 ******************************************************************************/
time_t
get_standard_time( void)
{
    time_t t;

    int jd, k, m, n, mm;
    double days, fjd;

    days = (double)time.weeks * 7.0;
    days += time.seconds / 86400.0;

    jd = (int)days + 2444244;       /* Integer part of Julian date */
    fjd = days - (int)days + 1.0;   /* Fractional part of the day */

    while( fjd >= 1.0)
    {
        jd += 1;
        fjd -= 1.0;
    }

    fjd *= 24.0; /* Hours */
    t.hours = (int)fjd;

    fjd = (fjd-(double)t.hours)*60.0;  /* Minutes */
    t.minutes = (int)fjd;

    t.seconds = (fjd-(double)t.minutes)*60.0; /* Seconds */

    k = jd + 68569;
    n = 4 * k / 146097;

    k = k - (146097 * n + 3) / 4;
    m = 4000 * (k + 1) / 1461001;
    k = k - 1461 * m / 4 + 31;

    mm = 80 * k / 2447;
    t.days = k - 2447 * mm / 80; /* day */

    k = mm / 11;
    t.months = mm + 2 - 12 * k;  /* Month */
    t.years = 100 * (n - 49) + m + k;   /* Year */

    return( t);
}


#if 0 // Is this ever used?
gpstime_t DateTimeToGps(time_t t)
{
    gpstime_t g;

    int     y, m, mjd;
    double  jd, d, fod;

    if(t.month <= 2)
{
        y = t.year  - 1;
        m = t.month + 12;
}
    else
{
        y = t.year;
        m = t.month;
}

    jd = floor(365.25 * y) + floor(30.6001 * (m + 1))
            + t.day + t.hour / 24.0 - 679019.0;

    mjd = (int)floor(jd);
    fod = fmod(jd, 1.0) + t.min / 1440.0 + t.sec / 86400.0;

    d = jd - 44244.0;
    g.week = (int)floor(d/7.0);
    g.sec  = (d - (double)(g.week) * 7.0) * 86400.0
            + t.min * 60.0 + t.sec;

    return (g);
}
#endif
