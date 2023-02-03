#include <cyg/kernel/kapi.h>
#include "gps4020.h"

static cyg_interrupt tic1b_int;
static cyg_handle_t tic1b_int_handle;

#define CYGNUM_HAL_PRI_HIGH     0
#define TIC1B_TIMER_PERIOD      500000 // 500,000 us = 0.5s

// ---------------------------------------------------------------------------
// GP4020 TIC Timer functions
// ---------------------------------------------------------------------------

void initialize_tic1b (void)
{
    volatile struct _gps4020_timer *tc = (volatile struct _gps4020_timer *)GPS4020_TC1;

    // Start timer in "reload" mode to set counter value
    tc->tc[1].reload = TIC1B_TIMER_PERIOD;  // 0.5 seconds = blinky LED @ 1 Hz
    tc->tc[1].control = TC_CTL_IE | TC_CTL_MODE_RELOAD | TC_CTL_SCR_HALT | TC_CTL_HEP | TC_CLOCK_BASE; 
}

void start_tic1b (void)
{
    volatile struct _gps4020_timer *tc = (volatile struct _gps4020_timer *)GPS4020_TC1;
    
    tc->tc[1].control = TC_CTL_IE | TC_CTL_MODE_RELOAD | TC_CTL_SCR_COUNT | TC_CTL_HEP | TC_CLOCK_BASE;
}

// ---------------------------------------------------------------------------
// ISR/DSR Functions
// ---------------------------------------------------------------------------

cyg_uint32 tic1b_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    // Mask the interrupt until after we're done with the DSR

    cyg_interrupt_mask(vector);

    // Clear the interrupt
    
    cyg_interrupt_acknowledge(vector);

    // The ISR is done; start the DSR next

    return(CYG_ISR_HANDLED | CYG_ISR_CALL_DSR);
}

void tic1b_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    // Start the clock up again.

    start_tic1b();

    // Toggle the LED on the GPIO #0 pin for fun

    if (in32(GPS4020_GPIO_READ) & 1)
        clearbits32(GPS4020_GPIO_WRITE,1);
    else
        setbits32(GPS4020_GPIO_WRITE,1);

    // Unmask the interrupt.

    cyg_interrupt_unmask( vector );
}

// ---------------------------------------------------------------------------
// "main" for our program. Note that it falls through to the idle thread
// ---------------------------------------------------------------------------

void cyg_user_start(void)
{
    cyg_vector_t tic1b_vector = CYGNUM_HAL_INTERRUPT_TIC1B;
    cyg_priority_t tic1b_priority = CYGNUM_HAL_PRI_HIGH;

    // Initialize and start the GP4020 TIC1b timer

    initialize_tic1b();
    start_tic1b();

    // Create the interrupt

    cyg_interrupt_create(tic1b_vector, tic1b_priority, 0, &tic1b_isr, &tic1b_dsr, &tic1b_int_handle, &tic1b_int);

    // Attach the interrupt created to the vector.

    cyg_interrupt_attach(tic1b_int_handle);

    // Unmask the interrupt we just configured, and away we go!

    cyg_interrupt_unmask(tic1b_vector);
}
