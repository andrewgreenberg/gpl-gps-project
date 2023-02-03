// gp4020_io.h headers for gp4020_io.c
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#ifndef __GP4020_IO_H
#define __GP4020_IO_H

// Bit positions of the Diagnostic LEDs (All are located on the GPIO port)
#define LED0 0x01
#define LED1 0x02
#define LED2 0x04
#define LED3 0x08
#define LED4 0x10
#define LED5 0x20
#define LED6 0x40
#define LED7 0x80

cyg_uint32  in32( cyg_uint32 address);
cyg_uint16  in16( cyg_uint32 address);

void  out32( cyg_uint32 address, cyg_uint32 data);
void  setbits32( cyg_uint32 address, cyg_uint32 data);
void  clearbits32( cyg_uint32 address, cyg_uint32 data);

void  out16( cyg_uint32 address, cyg_uint16 data);
void  setbits16( cyg_uint32 address, cyg_uint16 data);
void  clearbits16( cyg_uint32 address, cyg_uint16 data);

#endif // __GP4020_IO_H
