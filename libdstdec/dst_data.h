#ifndef _CU_DST_DATA_H_INCLUDED
#define _CU_DST_DATA_H_INCLUDED

#include <cstdio>
#include "types.h"

int GetDSTDataPointer(StrData* SD, uint8_t** pBuffer);
int ResetReadingIndex(StrData* SD);
int ReadNextBitFromBuffer(StrData* SD, uint8_t* pBit);
int ReadNextNBitsFromBuffer(StrData* SD, int32_t* pBits, int32_t NrBits);
int ReadNextByteFromBuffer(StrData* SD, uint8_t* pByte);
int FillBuffer(StrData* SD, uint8_t* pBuf, int32_t Size);
int FIO_BitGetChrUnsigned(StrData* SD, int32_t Len, uint8_t* x);
int FIO_BitGetIntUnsigned(StrData* SD, int32_t Len, int32_t* x);
int FIO_BitGetIntSigned(StrData* SD, int32_t Len, int32_t* x);
int FIO_BitGetShortSigned(StrData* SD, int32_t Len, int16_t* x);
int get_in_bitcount(StrData* SD);
int CreateBuffer(StrData* SD, int32_t Size);
int DeleteBuffer(StrData* SD);

#endif

