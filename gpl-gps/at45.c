// AT45.c: Source file for accessing and controlling the Atmel AT45DB161B
// serial flash chip 
// Copyright (C) 2005  Frank Mathew 
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#include <cyg/kernel/kapi.h>
#include "bsio.h"
#include "at45.h"


static unsigned char pad_buffer[60];


unsigned char
AT45_status(void)
{
    unsigned char status = 0;
    
    bsio_read(0, 0xD7000000, 8, 0, 0, &status, 1);
    
    return status;
}

cyg_bool
AT45_ready(void)
{
    cyg_bool      flag   = 0;
    unsigned char status = 0;
    unsigned int  tries  = 0;
    
    for (tries = 0; tries < 32768; tries++)
    {
        status = AT45_status();
    
        if ((status & 0x80) != 0)
        {
            flag = 1;
            break;
        }
    }
    
    return flag;
}


cyg_bool
AT45_init(void)
{
    int i = 0;
    
    for (i = 0; i < 60; i++)
        pad_buffer[i] = 0;
    
    return 0;
}


cyg_bool
AT45_erase(void)
{
    unsigned int  block_nr = 0;
    cyg_bool      status   = 0;

    for (block_nr=0; block_nr < 512; block_nr++)
    {
        if (!AT45_ready())
        {
            status = 1;
            break;
        }
        bsio_write(0, (0x50 | (block_nr << 13)), 32, 0, 0, 0, 0);
    }
    
    return status;
}
#define AT45_PAGE_LENGTH    528
#define AT45_PAGE_COUNT        4096
#define AT45_BLOCK_COUNT    512

// AT45_write_full_pages uses the page write-through command to
// write entire pages at a time 
cyg_bool
AT45_write_full_page(int  page_nr,
                     char *write_buffer)
{
    long int command        = 0x00000000;
    cyg_bool status         = 0;
    
    if (!AT45_ready())
    {
        status = 1;
    }
    else
    {
        command = 0x82000000
                  | ((page_nr << 10) & 0x003FFC00);
    
        status = bsio_write(0, command, 32,
                            0, 0,
                            write_buffer, AT45_PAGE_LENGTH);
    }
    
    return status;
}

// AT45_write_partial_page uses three commands to
// write part of a single page: 
//     read a flash page into buffer #1
//     write data into buffer #1
//     write buffer #1 back to memory with erase

cyg_bool
AT45_write_partial_page(int  page_nr,
                        int  page_offset,
                        char *write_buffer,
                        int  length)
{
    long int command = 0x00000000;
    cyg_bool status  = 0;

    // Wait for the flash to be ready
    if (!AT45_ready())
    {
        status = 1;
    }
    else
    {
        // Get the page data into buffer #1

        command = 0x53000000
                  | ((page_nr << 10) & 0x003FFC00);
    
        status = bsio_write(0, command, 32,
                            0, 0,
                            0, 0);

        // Write the data into buffer #1
        if (!AT45_ready())
        {
            status = 1;
        }
        else
        {
            command = 0x84000000
                      | (page_offset & 0x000003FF);
        
            status = bsio_write(0, command, 32,
                                0, 0,
                                write_buffer, length);

            // Push buffer #1 back to the flash page
            if (!AT45_ready())
            {
                status = 1;
            }
            else
            {
                command = 0x83000000
                          | ((page_nr << 10) & 0x003FFC00);
            
                status = bsio_write(0, command, 32,
                                    0, 0,
                                    0, 0);
            }
        }
    }
    
    return status;
}

// AT45_write writes an arbitrary number of bytes to
// an arbitrary set of pages in the flash chip
//    the byte stream can start anywhere in a page and 
//    end anywhere in the same or other page.

cyg_bool
AT45_write(long int address, char *write_buffer, long int length)
{
    int      page_nr               = 0;
    int      starting_byte_in_page = 0;
    int      current_write_length  = 0;
    cyg_bool status                = 0;

    while ((length > 0) && (status = 0))
    {
        page_nr = address / AT45_PAGE_LENGTH;
        starting_byte_in_page = address - (page_nr * AT45_PAGE_LENGTH);

        // If the address does not start at the beginning of a page
        // then write it as a partial page
        if (starting_byte_in_page > 0)
        {
            current_write_length = (AT45_PAGE_LENGTH
                                    - starting_byte_in_page
                                    + 1);
            if (current_write_length > length)
                current_write_length = length;
        
            status = AT45_write_partial_page(page_nr,
                                             starting_byte_in_page,
                                             write_buffer,
                                             current_write_length);
        }
        else
        if (length >= AT45_PAGE_LENGTH)
        {
            current_write_length = AT45_PAGE_LENGTH;
            status = AT45_write_full_page(page_nr,
                                          write_buffer);
        }
        else
        {
            current_write_length = length;
            status = AT45_write_partial_page(page_nr,
                                             0,
                                             write_buffer,
                                                current_write_length);
        }
        
        address      += current_write_length;
        write_buffer += current_write_length;
        length       -= current_write_length;
    }
    
    return status;
}


cyg_bool 
AT45_read(long int address, char *read_buffer, long int length)
{
    int      max_read_length     = 1023;
    int      current_read_length = 0;
    long int command             = 0x00000000;
    long int page_nr             = 0;
    long int byte_in_page        = 0;
    cyg_bool status              = 0;

    while ((length > 0) && (status == 0))
    {
        if (!AT45_ready())
        {
            status = 1;
        }
        else
        {
            current_read_length = max_read_length;
            if (length < max_read_length)
                current_read_length = length;
    
            page_nr = address / AT45_PAGE_LENGTH;
            byte_in_page = address - (page_nr * AT45_PAGE_LENGTH);
    
    
            command = 0xE8000000
                      | ((page_nr << 10) & 0x003FFC00)
                      | (byte_in_page    & 0x000003FF);
    
            status = bsio_read(0, command, 32,
                               pad_buffer, 4,
                               read_buffer, current_read_length);
    
            address     += current_read_length;
            read_buffer += current_read_length;
            length      -= current_read_length;
        }
    }
    
    return status;
}

