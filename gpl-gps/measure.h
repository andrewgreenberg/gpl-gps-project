// measure.h: Header file for the measure.c file
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#ifndef __MEASURE_H
#define __MEASURE_H

#include "constants.h"
#include "time.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

// NONE

/*******************************************************************************
 * Declarations
 ******************************************************************************/

typedef struct // Raw data
{
    unsigned short valid;      // This channel's measurement is valid
    unsigned short prn;

    unsigned short epoch_1ms;
    unsigned short epoch_20ms;

    unsigned short code_phase;
    unsigned short code_dco_phase;

    unsigned short carrier_dco_phase;
    unsigned short missed;         // Number of measurements (TIC's) missed

    unsigned long  carrier_cycles;

    long           doppler;
    unsigned long  meas_bit_time;

} measurement_t;

void grab_bit_times( void);
void measure_thread( CYG_ADDRWORD data);

/*******************************************************************************
 * Externs
 ******************************************************************************/

extern measurement_t meas[N_CHANNELS];
extern gpstime_t     meas_time;
extern cyg_sem_t     measure_semaphore;

#endif // __MEASURE_H
