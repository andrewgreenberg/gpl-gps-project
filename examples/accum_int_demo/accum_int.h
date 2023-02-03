// accum_ints.h: Header file for the accum_ints.c file

#ifndef __ACCUM_INT_H
#define __ACCUM_INT_H

void        initialize_accum_int    (void);
cyg_uint32  accum_isr               (cyg_vector_t vector, cyg_addrword_t data);
void        accum_dsr               (cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

#endif // __ACCUM_INT_H
