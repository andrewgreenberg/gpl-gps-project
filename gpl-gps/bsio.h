// bsio.h: Header file for the bsio.c file
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#ifndef __BSIO_H
#define __BSIO_H

typedef enum {
    SPI_MODE_0 = 0,
    SPI_MODE_1 = 1,
    SPI_MODE_2 = 2,
    SPI_MODE_3 = 3
} spi_mode;

typedef struct {
    unsigned long int write_errors;
    unsigned long int read_errors;
} bsio_errors;


cyg_bool bsio_initialize(spi_mode mode, int clock_freq);

cyg_bool bsio_write(int chip_select, 
                    long int control_word, int control_length,
                    char *write1_data, int write1_length,
                    char *write_data, int write_length);

cyg_bool bsio_read(int chip_select, 
                   long int control_word, int control_length,
                   char *write1_data, int write1_length,
                   char *read_data, int read_length);

cyg_bool bsio_write_read(int chip_select, 
                         long int control_word, int control_length,
                         char *write1_data, int write1_length,
                         char *write_data, int write_length, 
                         char *read_data, int read_length);

bsio_errors* bsio_get_error_counts(void);

void bsio_reset_error_counts(void);


#endif // __BSIO_H
