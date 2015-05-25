#ifndef _CU_DST_MEMORY_H_INCLUDED
#define _CU_DST_MEMORY_H_INCLUDED

#include "types.h"
#include <stdlib.h>

void* dst_memcpy(void* dst, void* src, size_t size);
void* dst_memset(void* dst, int val, size_t size);

#endif
