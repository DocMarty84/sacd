#ifndef __DSTDECODER_H_INCLUDED
#define __DSTDECODER_H_INCLUDED

#include "types.h"
#include "dst_unpack.h"

int Init(DstDec* D, int NrChannels, int Fs44);
int Close(DstDec* D);
int Decode(DstDec* D, uint8_t* DSTFrame, uint8_t* DSDMuxedChannelData, int FrameCnt, int* FrameSize);

#endif
