// message.h: Header file for the message.c file
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#ifndef __EPHEMERIS_H
#define __EPHEMERIS_H

#include "constants.h" // N_CHANNELS, NUM_SUBFRAMES

/*******************************************************************************
 * Definitions
 ******************************************************************************/

// None

/*******************************************************************************
 * Structures
 ******************************************************************************/

typedef struct
{
    unsigned long word[NUM_WORDS];
} sf_copy_t;

typedef struct
{
    unsigned short have_sf;
    sf_copy_t      sf[NUM_SUBFRAMES];
} msg_copy_t;

typedef struct
{
    unsigned short  prn;            // which satellite we're talkin' about
    unsigned short  valid;           // Use me, I'm valid.

    unsigned short  iodc;       // Issue of Data: Clock     (10 bits)
    unsigned short  iode;       // Issue of Data: Ephemeris  (8 bits)
    unsigned short  ura;        // User Range Accuracy       (4 bits)
    unsigned short  health;     // Sat vehicle health        (6 bits)

    /* Subframe 1: Clock Corrections for delta t(sv)
     *             (2C) = Two's complement = signed number
     *             b = bits */

    double  tgd;        // T(gd): L1-L2 correction            (2C 8b * 2^-31)
    double  toc;        // t(oc): clock ref. time   (max = 604784, 16b * 2^4)
    double  af0;        // a(f0): Constant term              (2C 22b * 2^-31)
    double  af1;        // a(f1): Linear term                (2C 16b * 2^-43)
    double  af2;        // a(f2): Squared term                (2C 8b * 2^-55)

    /* Subframe 2: Ephemeris */

    double  crs;        // C(rs): SIN correction radius      (2C 16b * 2^ -5)
    double  dn;         // Delta n: mean motion difference   (2C 16b * 2^-43)
    double  ma;         // M(0): mean anomaly                (2C 32b * 2^-31)
    double  cuc;        // C(uc): COS correction latitude    (2C 16b * 2^-29)
    double  ecc;        // e: eccentricity of orbit   (max=0.03, 32b * 2^-33)
    double  cus;        // C(us): SIN correctoin latitude    (2C 16b * 2^-29)
    double  sqrtA;      // (A)^1/2: sqrt semimajor axis         (32b * 2^-19)
    double  toe;        // T(oe):ephemeris ref. time (max=604,784, 16b * 2^4)

    /* Subframe 3: ephemeris */

    double  cic;        // C(ic): COS correction inclination (2C 16b * 2^-29)
    double  w0;         // Omega(0): longitude of asc. node  (2C 32b * 2^-31)
    double  cis;        // C(is): SIN correction inclination (2C 16b * 2^-29)
    double  inc0;       // I(0): inclination angle @ ref.    (2C 32b * 2^-31)
    double  crc;        // C(rc): COS correction radius      (2C 16b * 2^ -5)
    double  w;          // Omega: arguemnt of perigee        (2C 32b * 2^-31)
    double  omegadot;   // OMEGADOT: rate of right asc.      (2C 24b * 2^-43)
    double  idot;       // IDOT: rate of inclination angle   (2C 14b * 2^-43)

} ephemeris_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void ephemeris_thread( CYG_ADDRWORD data);
void clear_ephemeris( unsigned short ch);

/*******************************************************************************
 * Externs
 ******************************************************************************/

extern ephemeris_t  ephemeris[N_CHANNELS];

extern unsigned char log_eph[N_CHANNELS];

extern cyg_flag_t   ephemeris_channel_flag;
extern cyg_flag_t  ephemeris_subframe_flags[N_CHANNELS];

#endif // __EPHEMERIS_H
