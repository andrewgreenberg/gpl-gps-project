// serial.h: Header file for the serial.c file
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#ifndef __SERIAL_H
#define __SERIAL_H

void     uart2_initialize( void);
void     uart2_send_string( char * string);
cyg_bool uart2_get_char( cyg_uint8 *c);

#endif // __SERIAL_H
