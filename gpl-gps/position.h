// poition.h: Header file for the position.c file
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#ifndef __POSITION_H
#define __POSITION_H

#include "constants.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

// NONE?

/*******************************************************************************
 * Declarations
 ******************************************************************************/

typedef struct
{
    double az;
    double el;
} azel_t;

typedef struct
{
    double lat;
    double lon;
    double hgt;
} llh_t;

typedef struct
{
    double x, y, z;
} xyz_t;

typedef struct
{
    double  x, y, z;
    double  tb;
} xyzt_t;

typedef struct
{
    double  x, y, z;
    double  tb;
    unsigned short channel;
    unsigned short prn;
} satpos_t;

typedef struct
{
    double x,y,z,b;             // position [m] and time bias [m AND s]
//     double xv,yv,zv,df;
    double error;               // Convergence error from position calculation
    unsigned short valid;       // if the calculation is valid or not.
    unsigned short positioning; // If we're running the position calculation
    unsigned short n;           // number of satellites used in the solution
} position_t;

void position_thread( CYG_ADDRWORD data);
void clear_position( void);

/*******************************************************************************
 * Externs
 ******************************************************************************/

extern cyg_sem_t position_semaphore;

extern unsigned short pr2_data_fresh;

extern position_t   receiver_pos;
extern llh_t        receiver_llh;

extern azel_t   sat_azel[N_CHANNELS];
extern xyzt_t	sat_pos_by_ch[N_CHANNELS];

extern satpos_t sat_position[N_CHANNELS];
extern double   m_rho[N_CHANNELS];

#endif // __POSITION_H
