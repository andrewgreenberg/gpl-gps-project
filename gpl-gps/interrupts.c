// interrupts.c GP4020 accumulator/TIC interrupt functions
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.
#include <cyg/kernel/kapi.h>
#include "interrupts.h"
#include "constants.h"
#include "gp4020.h"
#include "gp4020_io.h"
#include "measure.h"
#include "time.h"
#include "tracking.h"

/******************************************************************************
 * Initialize the accumulator interrupt, ISR and DSR.
 ******************************************************************************/

void
initialize_gp4020_interrupts( void)
{
    // Enabled accum_int and meas_int interrupts; reading
    // ACCUM_STATUS_A will clear the accum_int, reading
    // ***MEAS***_STATUS_A will clear the meas_int.

    out16( GP4020_CORR_SYSTEM_SETUP, GCSS_INTERRUPT_ENABLE);
}


/******************************************************************************
 * Interrupt service routine for the accumulator interrupt (accum_int). Gets
 * called by the scheduler right after the ISR (which comes every every 505 us).
 ******************************************************************************/

cyg_uint32
accum_isr( cyg_vector_t vector, cyg_addrword_t data)
{
    setbits32( GPS4020_GPIO_WRITE, LED1); // Debug

    // Mask the interrupt until after we're done with the DSR
    cyg_interrupt_mask( vector);

    // Clear the interrupt
    cyg_interrupt_acknowledge( vector);

    // Clear the interrupt flag in the correlator and save the bits for
    // the tracking routine in a global variable. Also store the b
    // register for the tracking loop below
    accum_status_a = in16( GP4020_CORR_ACCUM_STATUS_A);
    accum_status_b = in16( GP4020_CORR_ACCUM_STATUS_B);

    // The ISR is done; start the DSR next
    return( CYG_ISR_HANDLED | CYG_ISR_CALL_DSR);
}

/******************************************************************************
 * "Delayed" service routine for the accumulator interrupt (accum_int). Gets
 * called by the scheduler right after the ISR (which comes every every 505 us).
 ******************************************************************************/

void
accum_dsr( cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    static unsigned short led_count;

    // Go track the satellite signals
    tracking();

    // If the TIC bit is set take a measurment
    if( accum_status_b & (1 << 13))
    {
        // Grab the time in bits
        grab_bit_times();

        // Wake up the mesaure thread
        cyg_semaphore_post( &measure_semaphore);

        // Heartbeat on LED 0: Toggle the LED on the GPIO #0 pin @ 2Hz
        if( ++led_count >= 5)
        {
            led_count = 0;

            if( in32( GPS4020_GPIO_READ) & 1)
                clearbits32( GPS4020_GPIO_WRITE, LED0); // DEBUG
            else
                setbits32( GPS4020_GPIO_WRITE, LED0);   // DEBUG
        }
    }

    clearbits32( GPS4020_GPIO_WRITE, LED1);    // DEBUG: Clear LED 1

    // Unmask the interrupt.
    cyg_interrupt_unmask( vector);
}
