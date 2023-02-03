// gp4020_in_out.c
#include <cyg/kernel/kapi.h>
#include "gp4020_in_out.h"

//==========================================================================
// 32 bit access functions
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

//==========================================================================
// 16 bit access functions
//==========================================================================

inline void out16 (cyg_uint32 address, cyg_uint16 data)
{
     *((volatile cyg_uint16 *)address) = data;
}

inline cyg_uint16 in16 (cyg_uint32 address)
{
    return( *((volatile cyg_uint16 *)address));
}

inline void setbits16 (cyg_uint32 address, cyg_uint16 data)
{
    *((volatile cyg_uint16 *)address) |= data;
}

inline void clearbits16 (cyg_uint32 address, cyg_uint16 data)
{
    *((volatile cyg_uint16 *)address) &= ~data;
}

