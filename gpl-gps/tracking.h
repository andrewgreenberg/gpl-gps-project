// tracking.h header file for tracking.c
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#ifndef __TRACKING_H
#define __TRACKING_H

#include "constants.h" // N_CHANNELS

/*******************************************************************************
 * Definitions
 ******************************************************************************/

// Clock info
#define SYSTEM_CLOCK  40E6                  /* nominal [Hz] */
#define SAMPLE_CLOCK  (SYSTEM_CLOCK / 7.0)  /* sets correlator timing */

// Carrier DCO timing
/* L1 carrier is down converted by the GP2015 front end to an IF frequency
 * of 1.4053968254MHz */
#define IF_FREQ   (SAMPLE_CLOCK - (L1 - 7 * SYSTEM_CLOCK * (5 + 1/2.0 + 1/9.0)))

#define CARR_FREQ_RES (SAMPLE_CLOCK / (1 << 27)) /* 42.574746268e-3 [mHz]
                                                   (Carrier DCO is 27 bits) */

/* Find integer setting used to tune the carrier DCO to the nominal frequency
 * (0x01f7b1b9) */
#define CARRIER_REF   (unsigned int)(0.5 + IF_FREQ / CARR_FREQ_RES)

#define CODE_FREQ_RES (SAMPLE_CLOCK / (1 << 26)) /* 85.149492536e-3 [mHz]
                                                   (Carrier DCO is 26 bits) */
/* Find integer setting used to tune the code DCO to the nominal chip rate
 * (0x016ea4a8)
 * Note, this is twice the expected value since the code generator works
 * in half-chips. This also means the chip resolution is actually two times
 * better than the value given in CODE_FREQ_RES. */
#define CODE_REF      (unsigned int)(0.5 + 2 * CHIP_RATE / CODE_FREQ_RES)

// Noise floor defines
#define NOISE_FLOOR_AVG 6619L
#define SIGNAL_LOSS_AVG 9349L

typedef enum {
    CHANNEL_OFF         = 0,
    CHANNEL_ON          = 1,
    CHANNEL_ACQUISITION = 1,
    CHANNEL_CONFIRM     = 2,
    CHANNEL_PULL_IN     = 3,
    CHANNEL_LOCK        = 4
} TRACKING_ENUM;

/*******************************************************************************
 * Structures
 ******************************************************************************/

typedef struct
{
    TRACKING_ENUM       state;
    unsigned short      prn;
    signed short        i_prompt, q_prompt; // Prompt arms (signed!)
    signed short        i_dither, q_dither; // Track arms (signed!)
    long                carrier_corr;
    long                carrier_freq;       // in DCO hex units
    long                code_freq;          // in DCO hex units
    signed short        n_freq;             // Carrier frequency search bin
    signed short        del_freq;           // Frequency search delta
    unsigned short      codephase;          // Current code phase
                                           // (in 1/2 chips, 0 - 2044)
    long                i_confirm;          //
    long                n_thresh;           //
    unsigned short      missed;             // # of missed accumulator dumps
    unsigned long       prompt_mag;         //
    unsigned long       dither_mag;         //
    unsigned short      ch_time;
    long                sum;
    long                th_rms;
    long                delta_code_freq, delta_code_freq_old;
    long                dcarr, dcarr1;
    long                old_theta;
    unsigned long       ms_sign;
    unsigned short      ms_count;
    long                avg;
    unsigned short      check_average;

    signed long         i_prompt_20 ,q_prompt_20;
    signed long         i_dither_20 ,q_dither_20;

    unsigned short      bit_sync;
    unsigned short      bit;

    unsigned short      clear_message;
    unsigned short      missed_message_bit;

    unsigned short      load_1ms_epoch_count;   // Flags to coordinate loading
    unsigned short      sync_20ms_epoch_count;  // the epoch counters.

    unsigned long       time_in_bits;           // Time of week, in bits
} chan_t;

/*******************************************************************************
 * Prototypes (Globally visible functions)
 ******************************************************************************/

// Called from gpl-gps.c
void tracking( void);
void initialize_tracking( void);

// called from allocate.c
void set_code_dco_rate( unsigned long ch, unsigned long freq);
void set_carrier_dco_rate( unsigned short ch, unsigned long freq);
void channel_power_control( unsigned short ch, unsigned short on);

/*******************************************************************************
 * Externs (globally visible variables)
 ******************************************************************************/

extern chan_t CH[N_CHANNELS];
extern unsigned short accum_status_a;
extern unsigned short accum_status_b;

#endif // __TRACKING_H

