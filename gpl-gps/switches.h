// switches.h: Header file for #defines to turn on and off features
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#ifndef __SWITCHES_H
#define __SWITCHES_H

// Define which logs to print
#define ENABLE_EPHEMERIS_LOG
#define ENABLE_POSITION_LOG
//#define ENABLE_PSEUDORANGE_LOG

// Define which display screens to offer
// (see display_thread() for hot-keys)
#define ENABLE_DEBUG_DISPLAY
#define ENABLE_EPHEMERIS_DISPLAY
#define ENABLE_MESSAGE_DISPLAY
#define ENABLE_POSITION_DISPLAY
#define ENABLE_PSEUDORANGE_DISPLAY
#define ENABLE_TRACKING_DISPLAY

#endif // __SWITCHES_H
