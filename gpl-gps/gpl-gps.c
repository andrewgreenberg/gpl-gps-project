// gpl-gps.c setup and mainline for gpl-gps
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.
#include <cyg/kernel/kapi.h>
#include "allocate.h"
#include "constants.h"
#include "display.h"
#include "ephemeris.h"
#include "gp4020.h"
#include "gp4020_io.h"
#include "interrupts.h"
#include "measure.h"
#include "message.h"
#include "position.h"
#include "pseudorange.h"
#include "time.h"
#include "tracking.h"

#define CYGNUM_HAL_PRI_HIGH         0

#define MEASURE_THREAD_PRIORITY     10
#define MESSAGE_THREAD_PRIORITY     12
#define EPHEMERIS_THREAD_PRIORITY   14
#define ALLOCATE_THREAD_PRIORITY    16
#define PSEUDORANGE_THREAD_PRIORITY 18
#define DISPLAY_THREAD_PRIORITY     20
#define POSITION_THREAD_PRIORITY    22

// The stack sizes are basicall x2 larger than we need; see the 'd' display
// in display.c
#define ALLOCATE_THREAD_STACK_SIZE      1120
#define DISPLAY_THREAD_STACK_SIZE       4096
#define EPHEMERIS_THREAD_STACK_SIZE     1120
#define MEASURE_THREAD_STACK_SIZE       1120
#define MESSAGE_THREAD_STACK_SIZE       1120
#define POSITION_THREAD_STACK_SIZE      4096
#define PSEUDORANGE_THREAD_STACK_SIZE   1120 // NEEDS TO BE 2048 IF LOGGING
                                            // (handle this in switches?)

/******************************************************************************
 * Globals (externed in threads.h, mind you)
 ******************************************************************************/

cyg_handle_t     accum_int_handle;

cyg_handle_t     allocate_thread_handle;
cyg_handle_t     display_thread_handle;
cyg_handle_t     ephemeris_thread_handle;
cyg_handle_t     measure_thread_handle;
cyg_handle_t     message_thread_handle;
cyg_handle_t     pseudorange_thread_handle;
cyg_handle_t     position_thread_handle;


/******************************************************************************
 * "main" for our program. Note that it falls through to the idle thread
 ******************************************************************************/

void
cyg_user_start( void)
{
    // accum_int variables
    cyg_vector_t            accum_vector = CYGNUM_HAL_INTERRUPT_CORR_ACCUM;
    cyg_priority_t          accum_priority = CYGNUM_HAL_PRI_HIGH;
    static cyg_interrupt    accum_int;

    // Message thread variables
    static cyg_thread       message_thread_obj;
    static int              message_stack[MESSAGE_THREAD_STACK_SIZE];

    // Measure thread variables
    static cyg_thread       measure_thread_obj;
    static int              measure_stack[MEASURE_THREAD_STACK_SIZE];

    // Allocate thread variables
    static cyg_thread       allocate_thread_obj;
    static int              allocate_stack[ALLOCATE_THREAD_STACK_SIZE];

    // Ephemeris thread variables
    static cyg_thread       ephemeris_thread_obj;
    static int              ephemeris_stack[EPHEMERIS_THREAD_STACK_SIZE];

    // pseudorange thread variables
    static cyg_thread       pseudorange_thread_obj;
    static int              pseudorange_stack[PSEUDORANGE_THREAD_STACK_SIZE];

    // position thread variables
    static cyg_thread       position_thread_obj;
    static int              position_stack[POSITION_THREAD_STACK_SIZE];

    // Display thread variables
    static cyg_thread       display_thread_obj;
    static int              display_stack[DISPLAY_THREAD_STACK_SIZE];

    // First, make sure the MPC Area 3 is set correctly:
    out32( GPS4020_MPC_AREA_1, GPS4020_MPC_A1_DEF);
    out32( GPS4020_MPC_AREA_2, GPS4020_MPC_A2_DEF);
    out32( GPS4020_MPC_AREA_3, GPS4020_MPC_A3_DEF);
    out32( GPS4020_MPC_AREA_4, GPS4020_MPC_A4_DEF);

    // Clear all the LEDs
    out32( GPS4020_GPIO_WRITE, 0);

    // Initialize threads

    // Initialize the message thread
    cyg_thread_create( MESSAGE_THREAD_PRIORITY, message_thread, 0,
        "message_thread", &message_stack, MESSAGE_THREAD_STACK_SIZE,
        &message_thread_handle, &message_thread_obj);
    cyg_thread_resume( message_thread_handle);

    // Initialize the measure thread
    cyg_thread_create( MEASURE_THREAD_PRIORITY, measure_thread, 0,
        "measure_thread", &measure_stack, MEASURE_THREAD_STACK_SIZE,
        &measure_thread_handle, &measure_thread_obj);
    cyg_thread_resume( measure_thread_handle);

    // Initialize the allocate thread
    cyg_thread_create(ALLOCATE_THREAD_PRIORITY, allocate_thread, 0,
        "allocate_thread", &allocate_stack, ALLOCATE_THREAD_STACK_SIZE,
        &allocate_thread_handle, &allocate_thread_obj);
    cyg_thread_resume( allocate_thread_handle);

    // Initialize the ephemeris thread
    cyg_thread_create( EPHEMERIS_THREAD_PRIORITY, ephemeris_thread, 0,
        "ephemeris_thread", &ephemeris_stack, EPHEMERIS_THREAD_STACK_SIZE,
        &ephemeris_thread_handle, &ephemeris_thread_obj);
    cyg_thread_resume( ephemeris_thread_handle);

    // Initialize the pseudorange thread
    cyg_thread_create( PSEUDORANGE_THREAD_PRIORITY, pseudorange_thread, 0,
        "pseudorange_thread", &pseudorange_stack, PSEUDORANGE_THREAD_STACK_SIZE,
        &pseudorange_thread_handle, &pseudorange_thread_obj);
    cyg_thread_resume( pseudorange_thread_handle);

    // Initialize the position thread
    cyg_thread_create( POSITION_THREAD_PRIORITY, position_thread, 0,
        "position_thread", &position_stack, POSITION_THREAD_STACK_SIZE,
        &position_thread_handle, &position_thread_obj);
    cyg_thread_resume( position_thread_handle);

    // Initialize the display thread
    cyg_thread_create( DISPLAY_THREAD_PRIORITY, display_thread, 0,
        "display_thread", &display_stack, DISPLAY_THREAD_STACK_SIZE,
        &display_thread_handle, &display_thread_obj);
    cyg_thread_resume( display_thread_handle);


    // Initialize the various modules
    initialize_tracking();
    initialize_allocation();

    // Initialize interrupts

    // Create the accum_int interrupt
    cyg_interrupt_create( accum_vector, accum_priority, 0, &accum_isr,
    	&accum_dsr, &accum_int_handle, &accum_int);
    cyg_interrupt_attach( accum_int_handle);
    cyg_interrupt_unmask( accum_vector);

    // Initialize the GP4020 correlator interrupts
    initialize_gp4020_interrupts();

    // Note that the global interrupt mask isn't cleared until the scheduler is
    // called after cyg_user_start exits.
}
