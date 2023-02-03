// meas_ints.h: Header file for the meas_ints.c file

#ifndef __MEAS_INT_H
#define __MEAS_INT_H

void        initialize_meas_int (void);
cyg_uint32  meas_isr            (cyg_vector_t vector, cyg_addrword_t data);
void        meas_dsr            (cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

#endif // __MEAS_INT_H
