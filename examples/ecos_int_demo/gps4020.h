//==========================================================================
//
//      gps4020.h
//
//      GPS-4020 Platform specific registers, etc
//
//==========================================================================

#ifndef _GPS4020_H_
#define _GPS4020_H_

//==========================================================================
// GP4020 Register address definitions
//==========================================================================

#define GPS4020_WATCHDOG    0xE0004000
#define GPS4020_INTC        0xE0006000
#define GPS4020_TC1         0xE000E000
#define GPS4020_TC2         0xE000F000
#define GPS4020_UART1       0xE0018000
#define GPS4020_UART2       0xE0019000

//==========================================================================
// GP4020 GPIO defines (32 bit access)
//==========================================================================

#define GPS4020_GPIO_DIR    0xE0005000
#define GPS4020_GPIO_READ   0xE0005004
#define GPS4020_GPIO_WRITE  0xE0005008

//==========================================================================
// GP4020 Watchdog defines
//==========================================================================

#define GPS4020_WATCHDOG_RESET 0xECD9F7BD

#ifndef __ASSEMBLER__
struct _gps4020_watchdog {
    unsigned long control;
    unsigned long period;
    unsigned long current;  // Only accessible in TEST mode
    unsigned long reset;
};
#endif

#ifndef __ASSEMBLER__
externC void _gps4020_watchdog(bool is_idle);
#endif

//==========================================================================
// GP4020 UART defines
//==========================================================================

#ifndef __ASSEMBLER__
struct _gps4020_uart {
    unsigned char control;
    unsigned char _fill0[3];
    unsigned char mode;
    unsigned char _fill1[3];
    unsigned char baud;
    unsigned char _fill2[3];
    unsigned char status;
    unsigned char _fill3[3];
    unsigned char TxRx;
    unsigned char _fill4[3];
    unsigned char modem_control;
    unsigned char _fill5[3+8];
    unsigned char modem_status;
    unsigned char _fill6[3];
};
#endif

// Serial control register

#define SCR_MIE 0x80   // Modem interrupt enable
#define SCR_EIE 0x40   // Error interrupt enable
#define SCR_TIE 0x20   // Transmit interrupt enable
#define SCR_RIE 0x10   // Receive interrupt enable
#define SCR_FCT 0x08   // Flow type 0=>software, 1=>hardware
#define SCR_CLK 0x04   // Clock source 0=internal, 1=external
#define SCR_TEN 0x02   // Transmitter enabled
#define SCR_REN 0x01   // Receiver enabled

// Serial mode register

#define SMR_DIV(x)      ((x)<<4)// Clock divisor
#define SMR_STOP        0x08    // Stop bits 0=>one, 1=>two
#define SMR_STOP_1      0x00
#define SMR_STOP_2      0x08
#define SMR_PARITY      0x04    // Parity mode 0=>even, 1=odd
#define SMR_PARITY_EVEN 0x00
#define SMR_PARITY_ODD  0x04
#define SMR_PARITY_ON   0x02    // Parity checked
#define SMR_PARITY_OFF  0x00
#define SMR_LENGTH      0x01    // Character length
#define SMR_LENGTH_8    0x00
#define SMR_LENGTH_7    0x01

// Serial status register

#define SSR_MSS      0x80   // Modem status has changed
#define SSR_OE       0x40   // Overrun error
#define SSR_FE       0x20   // Framing error
#define SSR_PE       0x10   // Parity error
#define SSR_TxActive 0x08   // Transmitter is active
#define SSR_RxActive 0x04   // Receiver is active
#define SSR_TxEmpty  0x02   // Tx buffer is empty
#define SSR_RxFull   0x01   // Rx buffer contains data

// Modem control register

#define SMR_CFG      0x08   // Configuration 0=>normal, 1=>null
#define SMR_MSU      0x04   // Modem status update 1=>enable
#define SMR_DTR      0x02   // Assert DTR
#define SMR_RTS      0x01   // Assert RTS

//==========================================================================
// GP4020 Timer defines
//==========================================================================

#ifndef __ASSEMBLER__
struct _gps4020_timer {
    struct {
        unsigned long control;
        unsigned long reload;
        unsigned long current;
        unsigned long _reserved[5];
    } tc[2];
};
#endif

// Timer/counter control
#define TC_CTL_IE           (1<<22)  // Interrupt enable
#define TC_CTL_OS           (1<<21)  // Overflow (count through 0)
#define TC_CTL_MODE         (3<<19)  // Timer/counter mode
#define TC_CTL_MODE_HALT    (0<<19)
#define TC_CTL_MODE_FREE    (1<<19)
#define TC_CTL_MODE_RELOAD  (2<<19)
#define TC_CTL_MODE_PWM     (3<<19)
#define TC_CTL_SCR          (1<<18)  // Software control request
#define TC_CTL_SCR_HALT     (0<<18)
#define TC_CTL_SCR_COUNT    (1<<18)
#define TC_CTL_HEP          (1<<17)  // Hardware enable polarity
#define TC_CTL_STAT         (1<<16)  // Current status

#define TC_CLOCK_BASE 20    // Assumes 20MHz system clock: 1/20 = 1 us

//==========================================================================
// GP4020 Interrupt Controller defines
//==========================================================================

#ifndef __ASSEMBLER__
struct _gps4020_intc {
    unsigned long sources;    // Active interrupt sources
    unsigned long polarity;   // 0=>active low
    unsigned long active;
    unsigned long trigger;    // 0=>level, 1=>edge
    unsigned long reset;      // reset edge triggers
    unsigned long enable;     // 1=>enable
    unsigned long status;     // masked (active and enabled)
    unsigned long type;       // 0=>IRQ, 1=>FIQ
    unsigned long FIQ_status;
    unsigned long IRQ_status;
    unsigned long FIQ_encoded;
    unsigned long IRQ_encoded;
};
#endif


//==========================================================================
// Memory/Register access functions
//==========================================================================

inline void out32 (cyg_uint32 address, cyg_uint32 data)
{
    *((volatile cyg_uint32 *)address) = data;
}

inline cyg_uint32 in32 (cyg_uint32 address)
{
    return( *((volatile cyg_uint32 *)address));
}

inline void setbits32 (cyg_uint32 address, cyg_uint32 data)
{
    *((volatile cyg_uint32 *)address) |= data;
}

inline void clearbits32 (cyg_uint32 address, cyg_uint32 data)
{
    *((volatile cyg_uint32 *)address) &= ~data;
}


#endif  // _GPS4020_H_
