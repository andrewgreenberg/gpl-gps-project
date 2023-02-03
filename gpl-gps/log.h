// log.h: Header file for the log.c file
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#ifndef ___LOG_H
#define ___LOG_H

#include "switches.h"

// NOTE: See switches.h for the log control defines.

#ifdef ENABLE_EPHEMERIS_LOG
void log_ephemeris( void);
#endif

#ifdef ENABLE_POSITION_LOG
void log_position( void);
#endif

#ifdef ENABLE_PSEUDORANGE_LOG
void log_pseudoranges( void);
#endif

#endif // ___LOG_H
