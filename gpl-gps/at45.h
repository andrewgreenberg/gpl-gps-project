// AT45.h: Header file for the AT45.c file
// Copyright (C) 2005  Frank Mathew 
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#ifndef __AT45_H
#define __AT45_H


// NOTE: For all functions a return value of 0 means OK.
//       A non-zero return value indicates an error
//       At present, the only error value is 1 -- general error 

cyg_bool AT45_init(void);

cyg_bool AT45_erase(void);

cyg_bool AT45_write(long int address, char *buffer, long int length);

cyg_bool AT45_read(long int address, char *buffer, long int length);

#endif // __AT45__ 

