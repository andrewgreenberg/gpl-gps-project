// interrupts.h: Header file for the interrupts.c file
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#ifndef __INTERRUPTS_H
#define __INTERRUPTS_H

void  initialize_gp4020_interrupts( void);

cyg_uint32 accum_isr( cyg_vector_t vector, cyg_addrword_t data);
void       accum_dsr( cyg_vector_t vector, cyg_ucount32 count,
                      cyg_addrword_t data);

#endif // __INTERRUPTS_H
