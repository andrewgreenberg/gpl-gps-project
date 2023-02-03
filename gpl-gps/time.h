// time.h header file for time.c
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#ifndef __TIME_H
#define __TIME_H

typedef enum{
    NO_CLOCK  = 0,  // This is the expected initialized startup state
    HOW_CLOCK = 1,  // Set from HOW, seconds accurate, week inaccurate
    SF1_CLOCK = 2,  // Set from SubFrame 1, the week number is correct
    FIX_CLOCK = 3   // Time corrected with position solution
} clock_state_t;

typedef struct
{
    unsigned short  weeks;
    double          seconds;
} gpstime_t;

typedef struct
{
    int years;
    int months;
    int days;
    int hours;
    int minutes;
    double seconds;
} time_t;

void increment_time_with_tic( void);
void set_time_with_tow( unsigned long tow);
void set_clock_correction( double correction);
void set_time_with_weeks( unsigned short weeks);
void set_to_fix_clock( void);

clock_state_t get_clock_state( void);
time_t        get_standard_time( void);
gpstime_t     get_time ( void);
double        get_time_in_seconds( void);

#endif // __TIME_H
