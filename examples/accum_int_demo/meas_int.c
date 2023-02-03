#include <cyg/kernel/kapi.h>
#include "gp4020.h"
#include "meas_int.h"
#include "gp4020_in_out.h"

// ---------------------------------------------------------------------------
// GP4020 meas_int functions
// ---------------------------------------------------------------------------

void initialize_meas_int (void)
{
    // Enable accum int and meas_int interrupts, choose ACCUM_STATUS_B to
    // clear the meas_int, use default accum_int period.
    
    out16(GP4020_CORR_SYSTEM_SETUP, SS_INTERRUPT_ENABLE);
}

// ---------------------------------------------------------------------------
// MEAS_INT ISR Functions
// ---------------------------------------------------------------------------

cyg_uint32 meas_isr (cyg_vector_t vector, cyg_addrword_t data)
{
    // Signal that the interrupt has occurred
    
    setbits32(GPS4020_GPIO_WRITE,8);

    // Clear the MEAS_INT interrupt flag in the correlator
    
    in16(GP4020_CORR_ACCUM_STATUS_B);
    
    // Mask the interrupt until after we're done with the DSR

    cyg_interrupt_mask(vector);

    // Clear the interrupt
    
    cyg_interrupt_acknowledge(vector);

    // The ISR is done; start the DSR next

    return(CYG_ISR_HANDLED | CYG_ISR_CALL_DSR);
}

// ---------------------------------------------------------------------------
// MEAS_INT ISR Functions
// ---------------------------------------------------------------------------

void meas_dsr (cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    static cyg_uint32   led_count;
    
    // Toggle the LED on the GPIO #2 pin @ 2Hz (= 1Hz flashing) for fun and profit
    // Note we've set tic1b to be 505 us (just like the accum_int will be)
    
    if (++led_count >= 10)
    {
        led_count = 0;
            
        if (in32(GPS4020_GPIO_READ) & 4)
            clearbits32(GPS4020_GPIO_WRITE,4);
        else
            setbits32(GPS4020_GPIO_WRITE,4);
    }

    // Signal that the meas_int ISR/DSR is done
        
    clearbits32(GPS4020_GPIO_WRITE,8);

    // Unmask the interrupt.

    cyg_interrupt_unmask( vector );
}
