// serial.c GP4020 polled serial drivers for UART2
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#include <cyg/kernel/kapi.h>
#include "serial.h"
#include "gp4020.h"

/******************************************************************************
 * Initialize UART2
 *
 * See GP4020 Design Manual section 17.2.1 for example baud rates. 
 * For 20MHz, 9600 = 129, 57600 = 15, 115200 = 10
 ******************************************************************************/
void
uart2_initialize( void)
{
    volatile struct _gps4020_uart *uart =
            (volatile struct _gps4020_uart *)GPS4020_UART2;

    uart->mode = SMR_STOP_1 | SMR_PARITY_OFF | SMR_LENGTH_8; // division = 1
    uart->baud = 10;
    uart->modem_control =  SMR_DTR | SMR_RTS;
    uart->control = SCR_TEN | SCR_REN;
}


/******************************************************************************
 * Send out a zero-terminated string on the serial line
 ******************************************************************************/
void
uart2_send_string( char * string)
{
    volatile struct _gps4020_uart *uart =
            (volatile struct _gps4020_uart *)GPS4020_UART2;

    while (*string)
    {
        while ((uart->status & SSR_TxEmpty) == 0)
            ;
        uart->TxRx = *string;
        string++;
    }
}


/******************************************************************************
 * Try to get a character; return true/false and the character if true->
 ******************************************************************************/
cyg_bool
uart2_get_char( cyg_uint8 *c)
{
    volatile struct _gps4020_uart *uart =
            (volatile struct _gps4020_uart *)GPS4020_UART2;

    if( (uart->status & SSR_RxFull) == 0)
        return 0;

    *c = uart->TxRx;
    return 1;
}
