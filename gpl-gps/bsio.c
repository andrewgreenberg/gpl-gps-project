// bsio.c: Source file for interfacing with the GP4020 BSIO serial
//         (SPI) interface.
// Copyright (C) 2005  Frank Mathew
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#include <cyg/kernel/kapi.h>
#include "bsio.h"

#define    BSIO_BASE_ADDRESS    0xE0007000
#define    GP4020_B_CLK         40000000    // assumed to match the system clock

//////////////////////////////////////////////////////////////////////////////////////////////////////
// These are bit masks for setting or clearing bits in the given BSIO register

#define    BSIO_CONFIG_SSEL_MASK       0x00038000
#define    BSIO_CONFIG_CONFSDIO_MASK   0x00000800
#define    BSIO_CONFIG_CONFSCLK_MASK   0x00000400
#define    BSIO_CONFIG_SCLKFREQ_MASK   0x00000380
#define    BSIO_CONFIG_SSLAG_MASK      0x00000060
#define    BSIO_CONFIG_SSLEAD_MASK     0x00000018
#define    BSIO_CONFIG_SCLKON_MASK     0x00000004

#define    BSIO_TRSFR_RXWORD_MASK      0x7C000000
#define    BSIO_TRSFR_TXWORD_MASK      0x03E00000
#define    BSIO_TRSFR_SELBYTE_MASK     0x00100000
#define    BSIO_TRSFR_WRSIZE_MASK      0x000FFC00
#define    BSIO_TRSFR_WRREM_MASK       0x000FFC00  // Yes, this is same as WRSIZE
#define    BSIO_TRSFR_RDSIZE_MASK      0x000003FF
#define    BSIO_TRSFR_RDREM_MASK       0x000003FF  // Yes, this is same as RDSIZE

#define    BSIO_MODE_CWORDSEL_MASK     0x00000020
#define    BSIO_MODE_CWORD_MASK        0x0000001F

#define    BSIO_SLAVE_TRFORMAT_MASK    0x00000020
#define    BSIO_SLAVE_CYCDELAY_MASK    0x00000010
#define    BSIO_SLAVE_CPOL_MASK        0x00000008
#define    BSIO_SLAVE_WRPOL_MASK       0x00000004
#define    BSIO_SLAVE_RDPOL_MASK       0x00000002
#define    BSIO_SLAVE_ENPOL_MASK       0x00000001

#define    BSIO_STAT_OPCOMP_MASK       0x00000080
#define    BSIO_STAT_OPERATION_MASK    0x00000040
#define    BSIO_STAT_WRITERR_MASK      0x00000020
#define    BSIO_STAT_READERR_MASK      0x00000010
#define    BSIO_STAT_VALBYTES_MASK     0x0000000C
#define    BSIO_STAT_RDREADY_MASK      0x00000002
#define    BSIO_STAT_WRREADY_MASK      0x00000001

#define    BSIO_INTC_OPCOMPEN_MASK     0x00000080
#define    BSIO_INTC_WRITERREN_MASK    0x00000020
#define    BSIO_INTC_READERREN_MASK    0x00000010
#define    BSIO_INTC_RDREADYEN_MASK    0x00000002
#define    BSIO_INTC_WRREADYEN_MASK    0x00000001

//////////////////////////////////////////////////////////////////////////////////////////////////////
// These are bit offsets for shifting a value from bit 0 to its correct place in the given BSIO register

#define    BSIO_CONFIG_SSEL_OFFSET       15        // 0x00038000
#define    BSIO_CONFIG_CONFSDIO_OFFSET   11        // 0x00000800
#define    BSIO_CONFIG_CONFSCLK_OFFSET   10        // 0x00000400
#define    BSIO_CONFIG_SCLKFREQ_OFFSET    7        // 0x00000380
#define    BSIO_CONFIG_SSLAG_OFFSET       5        // 0x00000060
#define    BSIO_CONFIG_SSLEAD_OFFSET      3        // 0x00000018
#define    BSIO_CONFIG_SCLKON_OFFSET      2        // 0x00000004

#define    BSIO_TRSFR_RXWORD_OFFSET      26        // 0x7C000000
#define    BSIO_TRSFR_TXWORD_OFFSET      21        // 0x03E00000
#define    BSIO_TRSFR_SELBYTE_OFFSET     20        // 0x00100000
#define    BSIO_TRSFR_WRSIZE_OFFSET      10        // 0x000FFC00
#define    BSIO_TRSFR_WRREM_OFFSET       10        // 0x000FFC00  // Yes, this is same as WRSIZE
#define    BSIO_TRSFR_RDSIZE_OFFSET       0        // 0x000003FF
#define    BSIO_TRSFR_RDREM_OFFSET        0        // 0x000003FF  // Yes, this is same as RDSIZE

#define    BSIO_MODE_CWORDSEL_OFFSET      5        // 0x00000020
#define    BSIO_MODE_CWORD_OFFSET         0        // 0x0000001F

#define    BSIO_SLAVE_TRFORMAT_OFFSET     5        // 0x00000020
#define    BSIO_SLAVE_CYCDELAY_OFFSET     4        // 0x00000010
#define    BSIO_SLAVE_CPOL_OFFSET         3        // 0x00000008
#define    BSIO_SLAVE_WRPOL_OFFSET        2         // 0x00000004
#define    BSIO_SLAVE_RDPOL_OFFSET        1        // 0x00000002
#define    BSIO_SLAVE_ENPOL_OFFSET        0         // 0x00000001

#define    BSIO_STAT_OPCOMP_OFFSET        7        // 0x00000080
#define    BSIO_STAT_OPERATION_OFFSET     6        // 0x00000040
#define    BSIO_STAT_WRITERR_OFFSET       5        // 0x00000020
#define    BSIO_STAT_READERR_OFFSET       4        // 0x00000010
#define    BSIO_STAT_VALBYTES_OFFSET      3        // 0x0000000C
#define    BSIO_STAT_RDREADY_OFFSET       1        // 0x00000002
#define    BSIO_STAT_WRREADY_OFFSET       0        // 0x00000001

#define    BSIO_INTC_OPCOMPEN_OFFSET      7        // 0x00000080
#define    BSIO_INTC_WRITERREN_OFFSET     6        // 0x00000020
#define    BSIO_INTC_READERREN_OFFSET     5        // 0x00000010
#define    BSIO_INTC_RDREADYEN_OFFSET     1        // 0x00000002
#define    BSIO_INTC_WRREADYEN_OFFSET     0        // 0x00000001


typedef struct {
    unsigned long config;
    unsigned long trsfr;
    unsigned long mode;
    unsigned long _reserved1;
    unsigned long slave0;
    unsigned long slave1;
    unsigned long _reserved2;
    unsigned long _reserved3;
    unsigned long _reserved4;
    unsigned long _reserved5;
    unsigned long _reserved6;
    unsigned long _reserved7;
    unsigned long stat;
    unsigned long intc;
    unsigned long rwbuf;
    unsigned long cwbuf;
} bsio_registers;

bsio_errors bsio_hold_errors = {0, 0};

struct {
    unsigned long   config;
    unsigned long   trsfr;
    unsigned long   mode;
    unsigned long   slave;
    unsigned long   last_stat;
    unsigned long   intc;
} bsio_hold[2] = {{ 0, 0, 0, 0, 0, 0},
                  { 0, 0, 0, 0, 0, 0}};

int last_ssel = 0;

long int imax(long int value1, long int value2)
{
    if (value1 > value2)
        return value1;
    else
        return value2;
}


/////////////////////////////////////////////////////////////
// This sets the CONFIG register shadow in the bsio_hold array.
// The array MUST STILL BE WRITTEN to the actual bsio device.
//////////////////////////////////////////////////////////////
void
bsio_set_config_register(long int ssel,
                         long int confsdio,
                         long int confsclk,
                         long int sclkfreq,
                         long int sslag,
                         long int sslead,
                         long int sclkon)
{
    long int temp      = 0;
    int      sclkratio = 512;

    // clear config register
    bsio_hold[ssel].config = 0;
    
    temp = (ssel << BSIO_CONFIG_SSEL_OFFSET) & BSIO_CONFIG_SSEL_MASK;
    bsio_hold[ssel].config |= temp;
    
    temp = (confsdio << BSIO_CONFIG_CONFSDIO_OFFSET) & BSIO_CONFIG_CONFSDIO_MASK;
    bsio_hold[ssel].config |= temp;
    
    temp = (confsclk << BSIO_CONFIG_CONFSCLK_OFFSET) & BSIO_CONFIG_CONFSCLK_MASK;
    bsio_hold[ssel].config |= temp;
    
    // Generate the frequency divider coding for the BSIO circuit
    sclkratio = imax((GP4020_B_CLK / sclkfreq), 512);

    if (sclkratio >= 512)
        temp = 8;
    else
    if (sclkratio >= 256)
        temp = 7;
    else
    if (sclkratio >= 128)
        temp = 6;
    else
    if (sclkratio >= 64)
        temp = 5;
    else
    if (sclkratio >= 32)
        temp = 4;
    else
    if (sclkratio >= 16)
        temp = 3;
    else
    if (sclkratio >= 8)
        temp = 2;
    else
    if (sclkratio >= 4)
        temp = 1;
    else
        temp = 0;

    temp = (temp << BSIO_CONFIG_SCLKFREQ_OFFSET) & BSIO_CONFIG_SCLKFREQ_MASK;
    bsio_hold[ssel].config |= temp;
    
    temp = ((sslag-1) << BSIO_CONFIG_SSLAG_OFFSET) & BSIO_CONFIG_SSLAG_MASK;
    bsio_hold[ssel].config |= temp;
    
    temp = ((sslead-1) << BSIO_CONFIG_SSLEAD_OFFSET) & BSIO_CONFIG_SSLEAD_MASK;
    bsio_hold[ssel].config |= temp;
    
    temp = (sclkon << BSIO_CONFIG_SCLKON_OFFSET) & BSIO_CONFIG_SCLKON_MASK;
    bsio_hold[ssel].config |= temp;
}

/////////////////////////////////////////////////////////////
// This initializes the trsfr register values to the bsio interface.
// Writing to this register initiates an IO action 
//////////////////////////////////////////////////////////////
void
bsio_write_trsfr_register(long int rxword,
                          long int txword,
                          long int selbyte,
                          long int wrsize,
                          long int rdsize
                          )
{
    long int temp =          0;
    long int hold_register = 0;
    bsio_registers* bsio   = (void *) BSIO_BASE_ADDRESS;
 
    temp = (rxword << BSIO_TRSFR_RXWORD_OFFSET) & BSIO_TRSFR_RXWORD_MASK;
    hold_register |= temp;
    
    temp = (txword << BSIO_TRSFR_TXWORD_OFFSET) & BSIO_TRSFR_TXWORD_MASK;
    hold_register |= temp;
    
    temp = (selbyte << BSIO_TRSFR_SELBYTE_OFFSET) & BSIO_TRSFR_SELBYTE_MASK;
    hold_register |= temp;
    
    temp = (wrsize << BSIO_TRSFR_WRSIZE_OFFSET) & BSIO_TRSFR_WRSIZE_MASK;
    hold_register |= temp;
    
    temp = (rdsize << BSIO_TRSFR_RDSIZE_OFFSET) & BSIO_TRSFR_RDSIZE_MASK;
    hold_register |= temp;
    
    // Write the transfer register
    bsio->trsfr = hold_register;
}

/////////////////////////////////////////////////////////////
// This writes the mode values to the bsio interface.
//////////////////////////////////////////////////////////////
void
bsio_write_mode_register(long int cwordsel,
                         long int cword
                         )
{
    long int        temp          = 0;
    long int        hold_register = 0;
    bsio_registers* bsio          = (void *)  BSIO_BASE_ADDRESS;
    
    temp = (cwordsel << BSIO_MODE_CWORDSEL_OFFSET) & BSIO_MODE_CWORDSEL_MASK;
    hold_register |= temp;
    
    temp = (cword << BSIO_MODE_CWORD_OFFSET) & BSIO_MODE_CWORD_MASK;
    hold_register |= temp;

    // Write the mode register
    bsio->mode = hold_register;
}

/////////////////////////////////////////////////////////////
// This initializes the slave value in the bsio_hold array.
// The array MUST STILL BE WRITTEN to the actual bsio device.
//////////////////////////////////////////////////////////////
void
bsio_set_slave_register(long int ssel,
                        long int trformat,
                        long int cycdelay,
                        long int cpol,
                        long int wrpol,
                        long int rdpol,
                        long int enpol
                        )
{
    long int temp = 0;
    
    // clear config register
    bsio_hold[ssel].slave = 0;
    
    temp = (trformat << BSIO_SLAVE_TRFORMAT_OFFSET) & BSIO_SLAVE_TRFORMAT_MASK;
    bsio_hold[ssel].slave |= temp;
    
    temp = (cycdelay << BSIO_SLAVE_CYCDELAY_OFFSET) & BSIO_SLAVE_CYCDELAY_MASK;
    bsio_hold[ssel].slave |= temp;
    
    temp = (cpol << BSIO_SLAVE_CPOL_OFFSET) & BSIO_SLAVE_CPOL_MASK;
    bsio_hold[ssel].slave |= temp;
    
    temp = (wrpol << BSIO_SLAVE_WRPOL_OFFSET) & BSIO_SLAVE_WRPOL_MASK;
    bsio_hold[ssel].slave |= temp;
    
    temp = (rdpol << BSIO_SLAVE_RDPOL_OFFSET) & BSIO_SLAVE_RDPOL_MASK;
    bsio_hold[ssel].slave |= temp;
    
    temp = (enpol << BSIO_SLAVE_ENPOL_OFFSET) & BSIO_SLAVE_ENPOL_MASK;
    bsio_hold[ssel].slave |= temp;
}


////////////////////////////////////////////////////////////
// Read the stat register and parse into usable numbers
////////////////////////////////////////////////////////////
int
bsio_stat_opcomp(int ssel)
{
    long int stat = bsio_hold[ssel].last_stat;
    
    return (stat & BSIO_STAT_OPCOMP_MASK) >> BSIO_STAT_OPCOMP_OFFSET;
}

int
bsio_stat_operation(int ssel)
{
    long int stat = bsio_hold[ssel].last_stat;
    
    return (stat & BSIO_STAT_OPERATION_MASK) >> BSIO_STAT_OPERATION_OFFSET;
}
    
int
bsio_stat_writerr(int ssel)
{
    long int stat = bsio_hold[ssel].last_stat;
    
    return (stat & BSIO_STAT_WRITERR_MASK) >> BSIO_STAT_WRITERR_OFFSET;
}
    
int
bsio_stat_readerr(int ssel)
{
    long int stat = bsio_hold[ssel].last_stat;
    
    return (stat & BSIO_STAT_READERR_MASK) >> BSIO_STAT_READERR_OFFSET;
}
    
int
bsio_stat_valbytes(int ssel)
{
    long int stat = bsio_hold[ssel].last_stat;
    
    return (stat & BSIO_STAT_VALBYTES_MASK) >> BSIO_STAT_VALBYTES_OFFSET;
}
    
int
bsio_stat_rdready(int ssel)
{
    long int stat = bsio_hold[ssel].last_stat;
    
    return (stat & BSIO_STAT_RDREADY_MASK) >> BSIO_STAT_RDREADY_OFFSET;
}
    
int
bsio_stat_wrready(int ssel)
{
    long int stat = bsio_hold[ssel].last_stat;
    
    return (stat & BSIO_STAT_WRREADY_MASK) >> BSIO_STAT_WRREADY_OFFSET;
}

int
bsio_trsfr_wrrem(int ssel)
{
    long int trsfr = bsio_hold[ssel].trsfr;
    
    return (trsfr & BSIO_TRSFR_WRREM_MASK) >> BSIO_TRSFR_WRREM_OFFSET;
}

int
bsio_trsfr_rdrem(int ssel)
{
    long int trsfr = bsio_hold[ssel].trsfr;
    
    return (trsfr & BSIO_TRSFR_RDREM_MASK) >> BSIO_TRSFR_RDREM_OFFSET;
}



/////////////////////////////////////////////////////////////
// This initializes the intc (interrupt control value in the 
// bsio_hold array.  The array MUST STILL BE WRITTEN to the 
// actual bsio device.
//////////////////////////////////////////////////////////////
void
bsio_set_intc_register(long int ssel,
                       long int opcompen,
                       long int writerren,
                       long int readerren,
                       long int rdreadyen,
                       long int wrreadyen
                       )
{
    long int temp = 0;
    
    // clear the interrupt control register
    bsio_hold[ssel].intc = 0;
    
    temp = (opcompen << BSIO_INTC_OPCOMPEN_OFFSET) & BSIO_INTC_OPCOMPEN_MASK;
    bsio_hold[ssel].intc |= temp;
    
    temp = (writerren << BSIO_INTC_WRITERREN_OFFSET) & BSIO_INTC_WRITERREN_MASK;
    bsio_hold[ssel].intc |= temp;
    
    temp = (readerren << BSIO_INTC_READERREN_OFFSET) & BSIO_INTC_READERREN_MASK;
    bsio_hold[ssel].intc |= temp;
    
    temp = (rdreadyen << BSIO_INTC_RDREADYEN_OFFSET) & BSIO_INTC_RDREADYEN_MASK;
    bsio_hold[ssel].intc |= temp;
    
    temp = (wrreadyen << BSIO_INTC_WRREADYEN_OFFSET) & BSIO_INTC_WRREADYEN_MASK;
    bsio_hold[ssel].intc |= temp;
}


void
bsio_write_registers(int ssel)
{
    bsio_registers* bsio = (void *) BSIO_BASE_ADDRESS;

    if ((ssel == 0) || (ssel ==1))
    {
        bsio->config = bsio_hold[ssel].config;            // transfer all controlling data from the appropriate
        bsio->mode   = bsio_hold[ssel].mode;
        if (ssel == 0)
        {
            bsio->slave0 = bsio_hold[ssel].slave;        // There are separate slave registers for the
        }                                                // two chip selects
        else
        {
            bsio->slave1 = bsio_hold[ssel].slave;
        }

        bsio->intc   = bsio_hold[ssel].intc;

        bsio_hold[ssel].last_stat = bsio->stat; // get back the current chip stat bits
    }
}

// typical use:  bsio_initialize(0, 156250);

cyg_bool
bsio_initialize(spi_mode mode, int clock_freq)
{
    int ssel  = 0;
    int cpol  = 0;
    int wrpol = 0;
    int rdpol = 0;
    int enpol = 0;
    
    
    // Setthe clock polarities for both slaves (for now)
    
    switch (mode)
    {
        case SPI_MODE_0:
            cpol  = 0;
            wrpol = 0;
            rdpol = 1;
            break;
        case SPI_MODE_1:
            cpol  = 0;
            wrpol = 1;
            rdpol = 0;
            break;
        case SPI_MODE_2:
            cpol  = 1;
            wrpol = 1;
            rdpol = 0;
            break;
        case SPI_MODE_3:
            cpol  = 1;
            wrpol = 0;
            rdpol = 1;
            break;
    }
    
    for (ssel = 0; ssel <= 1; ssel++)
    {
        bsio_set_config_register(ssel, 0, 0, clock_freq, 0, 0, 1);
        bsio_set_slave_register(ssel, 0, 0, cpol, wrpol, rdpol, enpol);
        bsio_set_intc_register(ssel, 0, 0, 0, 0, 0);
    }
    
    // Write the register block for chip-select 0 to the actual bsio registers
    bsio_write_registers(0);
    
    return 0;
}



cyg_bool
bsio_write(int      chip_select,
           long int control_word,
           int      control_bit_length,
           char     *write1_data,
           int      write1_length,
           char     *write_data,
           int      write_length)
{
    return bsio_write_read(chip_select,
                           control_word, control_bit_length,
                           write1_data, write1_length,
                           write_data, write_length,
                           0, 0);
}



cyg_bool
bsio_read(int      chip_select,
          long int control_word,
          int      control_bit_length,
          char     *write1_data,
          int      write1_length,
          char     *read_data,
          int      read_length
          )
{
    return bsio_write_read(chip_select,
                           control_word, control_bit_length,
                           write1_data, write1_length,
                           0, 0,
                           read_data, read_length);
}



cyg_bool
bsio_write_read(int      chip_select,
                long int control_word,
                int      control_length,
                char     *write1_data,
                int      write1_length,
                char     *write_data,
                int      write_length,
                char     *read_data,
                int      read_length
                )
{
    int             i                 = 0;
    int             write_data_count  = 0;
    int             write1_data_count = 0;
    int             read_data_count   = 0;
    long int        hold_data         = 0;
    int             valid_bytes       = 0;
    bsio_registers* bsio              = (void *) BSIO_BASE_ADDRESS;

    if (chip_select != last_ssel)
        bsio_write_registers(chip_select);

    if (((write_length + write1_length) > 1023) || (read_length > 1023))
        return 1;

    // First prime the "pump" with the control word and the first output word
    bsio->cwbuf = control_word;

    // Configure the control word transmission via the mode register
    bsio_write_mode_register(1, control_length);
    
    // Write up to 4 bytes
    hold_data = 0;
    for (i = 0; i < 4; i++)
    {
        if (i > 0)
            hold_data = hold_data << 8;

        if (write1_length > 0)
        {
            hold_data |= write1_data[write1_data_count++];
            write1_length--;
        }

        if ((write1_length <= 0) && (write_length > 0))
        {
            hold_data |= write_data[write_data_count++];
            write_length--;
        }
    }
    bsio->rwbuf = hold_data;
    
    // Start the data transfer by writing to the transfer register
    bsio_write_trsfr_register(32, 32, 1,
                              (write_length + write1_length), read_length);
    
    // keep the data pump full
    
    bsio_hold[chip_select].last_stat = bsio->stat;
    bsio_hold_errors.write_errors += bsio_stat_writerr(chip_select);
    
    bsio_hold[chip_select].trsfr = bsio->trsfr;
    
    // Continue while the write operation is in progress
    while (bsio_trsfr_wrrem(chip_select) > 0)
    {
        // if the bsio is ready for another word, then write it
        if (bsio_stat_wrready(chip_select) == 1)
        {
            // Write up to 4 bytes
            hold_data = 0;
            for (i = 0; i < 4; i++)
            {
                if (i > 0)
                    hold_data = hold_data << 8;

                if (write1_length > 0)
                {
                    hold_data |= write1_data[write1_data_count++];
                    write1_length--;
                }
                if ((write1_length <= 0) && (write_length > 0))
                {
                    hold_data |= write_data[write_data_count++];
                    write_length--;
                }
            }
            bsio->rwbuf = hold_data;
        }
    
        bsio_hold[chip_select].last_stat = bsio->stat;
        bsio_hold_errors.write_errors += bsio_stat_writerr(chip_select);

        bsio_hold[chip_select].trsfr = bsio->trsfr;
    }
    
    // Continue while the read operation is in progress
    while (bsio_trsfr_rdrem(chip_select) > 0)
    {
        if (bsio_stat_rdready(chip_select) == 1)
        {
            // read up to 4 bytes
            hold_data = bsio->rwbuf;
            valid_bytes = bsio_stat_valbytes(chip_select) + 1;
            while ((read_data_count < read_length) && (valid_bytes > 0))
            {
                read_data[read_data_count++] =
                            ((hold_data >> 24) & 0x000000FF);
                hold_data = hold_data << 8;
                valid_bytes--;
            }
        }
    
        bsio_hold[chip_select].last_stat = bsio->stat;
        bsio_hold_errors.read_errors += bsio_stat_readerr(chip_select);

        bsio_hold[chip_select].trsfr     = bsio->trsfr;
    }

    return 0;
}


bsio_errors*
bsio_get_error_counts()
{
    return &bsio_hold_errors;
}

void
bsio_reset_error_counts()
{
    bsio_hold_errors.write_errors = 0;
    bsio_hold_errors.read_errors = 0;
}
