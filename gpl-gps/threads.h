// allocate.h: Header file for the allocate.c file
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#ifndef __THREADS_H
#define __THREADS_H

/*******************************************************************************
 * Definitions
 ******************************************************************************/

// NONE

/*******************************************************************************
 * Declarations
 ******************************************************************************/

// None

/*******************************************************************************
 * Externs
 ******************************************************************************/

extern cyg_handle_t     message_thread_handle;
extern cyg_handle_t     measure_thread_handle;
extern cyg_handle_t     allocate_thread_handle;
extern cyg_handle_t     ephemeris_thread_handle;
extern cyg_handle_t     pseudorange_thread_handle;
extern cyg_handle_t     position_thread_handle;
extern cyg_handle_t     display_thread_handle;

// Not really a thread, but an interrupt
extern cyg_handle_t     accum_int_handle;

#endif // __THREADS_H
