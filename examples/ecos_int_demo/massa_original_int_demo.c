#include <cyg/kernel/kapi.h>

static cyg_interrupt int1;
static cyg_handle_t int1_handle;
static cyg_sem_t data_ready;

#define CYGNUM_HAL_INTERRUPT_1    1
#define CYGNUM_HAL_PRI_HIGH       0

//
// Interrupt service routine for interrupt 1.
//

cyg_uint32 interrupt_1_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    // Block this interrupt from occurring until
    // the DSR completes.

    cyg_interrupt_mask(vector);

    // Tell the processor that we have received
    // the interrupt.

    cyg_interrupt_acknowledge(vector);

    // Tell the kernel that chained interrupt processing
    // is done and the DSR needs to be executed next.

    return(CYG_ISR_HANDLED | CYG_ISR_CALL_DSR);
}

//
// Deferred service routine for interrupt 1.
//

void interrupt_1_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    // Signal the thread to run for further processing.

    cyg_semaphore_post( &data_ready );

    // Allow this interrupt to occur again.

    cyg_interrupt_unmask( vector );
}


//
// Main starting point for the application.
//

void cyg_user_start(void)
{
    cyg_vector_t int1_vector = CYGNUM_HAL_INTERRUPT_1;
    cyg_priority_t int1_priority = CYGNUM_HAL_PRI_HIGH;

    // Initialize the semaphore used for interrupt 1.

    cyg_semaphore_init(&data_ready, 0);

    //
    // Create interrupt 1.
    //
    //
    cyg_interrupt_create(int1_vector, int1_priority, 0, &interrupt_1_isr, &interrupt_1_dsr, &int1_handle, &int1);

    // Attach the interrupt created to the vector.

    cyg_interrupt_attach(int1_handle);

    // Unmask the interrupt we just configured.

    cyg_interrupt_unmask(int1_vector);
}
