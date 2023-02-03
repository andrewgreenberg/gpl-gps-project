// gp4020_in_out.h

#ifndef __GP4020_IN_OUT_H
#define __GP4020_IN_OUT_H

inline void out32 (cyg_uint32 address, cyg_uint32 data);
inline cyg_uint32 in32 (cyg_uint32 address);
inline void setbits32 (cyg_uint32 address, cyg_uint32 data);
inline void clearbits32 (cyg_uint32 address, cyg_uint32 data);

inline void out16 (cyg_uint32 address, cyg_uint16 data);
inline cyg_uint16 in16 (cyg_uint32 address);
inline void setbits16 (cyg_uint32 address, cyg_uint16 data);
inline void clearbits16 (cyg_uint32 address, cyg_uint16 data);

#endif // __GP4020_IN_OUT_H

