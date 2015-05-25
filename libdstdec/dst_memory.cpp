#include "dst_memory.h"

void* dst_memcpy(void* dst, void* src, size_t size) {
    for (size_t i = 0; i < size; i++) {
        *((uint8_t*)dst + i) = *((uint8_t*)src + i);
    }
    return dst;
}

void* dst_memset(void* dst, int val, size_t size) {
    for (size_t i = 0; i < size; i++) {
        *((uint8_t*)dst + i) = (uint8_t)val;
    }
    return dst;
}
