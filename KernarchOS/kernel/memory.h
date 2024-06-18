#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* memset(void* ptr, int value, size_t num);
void* memcpy(void* dest, const void* src, size_t num);
int memcmp(const void* ptr1, const void* ptr2, size_t num);

#ifdef __cplusplus
}
#endif

#endif // MEMORY_H