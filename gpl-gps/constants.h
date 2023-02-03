// constants.h: Header file for various SHARED constants
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#ifndef __CONSTANTS_H
#define __CONSTANTS_H

// ECEF location of our survey point in [m]
#include "here.h"

 // GP4020 Defines which might want to go in gp4020.h
#define N_CHANNELS     12
#define TIC_PERIOD  0.0999999 // This is the default = (100ms - 100ns)

// GPS Constellation

#define N_SATELLITES 32

// RF Timing
#define GPS_REF       10.23E6  /* theoretical GPS reference frequency [Hz] */
#define L1            (154 * GPS_REF)       /* nominal civilian GPS satellite
                                               carrier frequency, 1.57542GHz */

// Satellite Navigation Message
#define NUM_SUBFRAMES 5
#define NUM_WORDS 10

// Code DCO timing
#define CODE_DCO_LENGTH    1024
#define CODE_PERIOD        1023		      // Gold code period
#define MAX_CODE_PHASE     (2 * CODE_PERIOD)  // In half-chips
#define CHIP_RATE          (GPS_REF / 10)
#define CODE_TIME          (CODE_PERIOD / CHIP_RATE) // 1ms precisely
#define QUARTER_CHIP_TIME  (0.25 / CHIP_RATE) // about 74[m]

// GPS Week Defines
#define SECONDS_IN_WEEK  (7 * 24 * 3600)      // (604800) seconds per week
#define BITS_IN_WEEK  (50 * SECONDS_IN_WEEK)  // (30240000) gps 50Hz data rate

// Physical Constants
#define SPEED_OF_LIGHT  2.99792458E8       // [m/s]
#define PI              3.141592653589793
#define PI_OVER_2       (PI / 2.0)
#define TWO_PI          (PI * 2.0)
#define RADTODEG        180.0/PI           // [deg/r]

#endif // __CONSTANTS_H
