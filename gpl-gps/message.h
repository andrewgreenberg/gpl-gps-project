// message.h: Header file for the message.c file
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#ifndef __MESSAGE_H
#define __MESSAGE_H

#include "constants.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

// NONE

/*******************************************************************************
 * Declarations
 ******************************************************************************/

typedef struct
{
    unsigned long       word[10];
    unsigned long       TOW;            // "Truncated time Of Week"
                                        // That is, time since beginning of the
                                        // week in 6 second (subframe) increments
    unsigned long       valid;          // 10 bits of word validity flags;
                                        // all good = 0x3ff
 } subframe_t;

typedef struct
{
    unsigned short      prn;            // satellite prn

    unsigned short      frame_sync;
    unsigned short      set_ch_time;    // only set the time_in_bits and epoch
                                        // counter once per sync on a channel
    unsigned long       wordbuf0;       // Current word (2+30 format)
    unsigned long       wordbuf1;       // previous word (2+30 format)

    unsigned short      data_inverted;
    unsigned short      bitcount;       // # of bits in the current word
    unsigned short      wordcount;      // # of words in the subframe so far

    unsigned short      subframe;       // Current subframe #
    subframe_t          sf[5];          // Array of 5 subframes (1 frame)
} message_t;

void message_thread( CYG_ADDRWORD data);
void clear_messages( unsigned short ch);

/*******************************************************************************
 * Externs
 ******************************************************************************/

extern cyg_flag_t message_flag;
extern message_t  messages[N_CHANNELS];

#endif // __MESSAGE_H
