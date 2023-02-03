// display.h: Header file for the display.c file
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#ifndef __DISPLAY_H
#define __DISPLAY_H

typedef enum {
    DISPLAY_TRACKING    = 0,
    DISPLAY_MESSAGES    = 1,
    DISPLAY_EPHEMERIS   = 2,
    DISPLAY_PSEUDORANGE = 3,
    DISPLAY_POSITION    = 4,
    DISPLAY_STOP        = 5,
    DISPLAY_DEBUG       = 6,
    DISPLAY_LOG         = 7,
    DISPLAY_NOT_USED,       // Used to force clear screen on startup
} display_t;

void display_thread( CYG_ADDRWORD data);

extern unsigned short display_command;

#endif // __DISPLAY_H
