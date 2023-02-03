#include <cyg/kernel/kapi.h>
#include "gp4020.h"
#include "gp4020_in_out.h"
#include "accum_int.h"
#include "meas_int.h"

#define CYGNUM_HAL_PRI_HIGH     0

// ---------------------------------------------------------------------------
// "main" for our program. Note that it falls through to the idle thread
// ---------------------------------------------------------------------------

void cyg_user_start(void)
{
    cyg_vector_t            accum_vector = CYGNUM_HAL_INTERRUPT_CORR_ACCUM;
    cyg_priority_t          accum_priority = CYGNUM_HAL_PRI_HIGH;
    static cyg_interrupt    accum_int;
    static cyg_handle_t     accum_int_handle;
    
    cyg_vector_t            meas_vector = CYGNUM_HAL_INTERRUPT_CORR_MEAS;
    cyg_priority_t          meas_priority = CYGNUM_HAL_PRI_HIGH;
    static cyg_interrupt    meas_int;
    static cyg_handle_t     meas_int_handle;

    // First, make sure the MPC Area 3 is set correctly:
    
    out32(GPS4020_MPC_AREA_1,GPS4020_MPC_A1_DEF);
    out32(GPS4020_MPC_AREA_2,GPS4020_MPC_A2_DEF);
    out32(GPS4020_MPC_AREA_3,GPS4020_MPC_A3_DEF);
    out32(GPS4020_MPC_AREA_4,GPS4020_MPC_A4_DEF);

    // Clear all the LEDs
    
    out32(GPS4020_GPIO_WRITE,0);

    // Initialize the GP4020 correlator interrupts (don't need to call initialize_meas_int)

    initialize_accum_int();
                
    // Create the accum_int interrupt

    cyg_interrupt_create(accum_vector, accum_priority, 0, &accum_isr, &accum_dsr, &accum_int_handle, &accum_int);
    cyg_interrupt_attach(accum_int_handle);
    cyg_interrupt_unmask(accum_vector);

    // Create the meas_int interrupt
        
    cyg_interrupt_create(meas_vector, meas_priority, 0, &meas_isr, &meas_dsr, &meas_int_handle, &meas_int);
    cyg_interrupt_attach(meas_int_handle);
    cyg_interrupt_unmask(meas_vector);
    
    // Note that the global interrupt mask isn't cleared until the scheduler is called 
    // after cyg_user_start exits.
}
