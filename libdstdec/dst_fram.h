#ifndef __DST_FRAM_H_INCLUDED
#define __DST_FRAM_H_INCLUDED

#include "types.h"

int DST_FramDSTDecode(DstDec* D, uint8_t* DSTdata, uint8_t* MuxedDSDdata, int FrameSizeInBytes, int FrameCnt);

#endif
