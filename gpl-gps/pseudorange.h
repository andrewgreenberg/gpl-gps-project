// pseudorange.h: Header file for the pseudorange.c file
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#ifndef __PSEUDORANGE_H
#define __PSEUDORANGE_H

#include "time.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

// NONE?

/*******************************************************************************
 * Declarations
 ******************************************************************************/

typedef struct
{
    unsigned short  valid;
    unsigned short  prn;
    double          sat_time;        // Time of transmission
    double          range;
    double          residual;

    unsigned long   bit_time;       // for debugging epoch counter stuff
    unsigned short  epoch_20ms;
    unsigned short  epoch_1ms;
    unsigned short  code_phase;
    unsigned short  code_dco_phase;
} pseudorange_t;

void pseudorange_thread( CYG_ADDRWORD data);
void clear_pseudoranges( void);

/*******************************************************************************
 * Externs
 ******************************************************************************/

extern cyg_flag_t pseudorange_flag;

extern pseudorange_t pr[N_CHANNELS];
extern gpstime_t     pr_time;

extern pseudorange_t pr2[N_CHANNELS];
extern gpstime_t     pr2_time;

#endif // __PSEUDORANGE_H
