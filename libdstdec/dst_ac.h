#ifndef __DST_AC_H_INCLUDED
#define __DST_AC_H_INCLUDED

#include "types.h"

void DST_ACDecodeBit(ACData* AC, uint8_t* b, int p, uint8_t* cb, int fs, int flush);
int DST_ACGetPtableIndex(long PredicVal, int PtableLen);

#endif
