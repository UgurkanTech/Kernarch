#include "memory.h"

void* memset(void* ptr, int value, size_t num) {
    unsigned char* ptr_char = (unsigned char*)ptr;
    while (num--) {
        *ptr_char++ = (unsigned char)value;
    }
    return ptr;
}

void* memcpy(void* dest, const void* src, size_t num) {
    char* dest_char = (char*)dest;
    const char* src_char = (const char*)src;
    while (num--) {
        *dest_char++ = *src_char++;
    }
    return dest;
}

int memcmp(const void* ptr1, const void* ptr2, size_t num) {
    const unsigned char* ptr1_char = (const unsigned char*)ptr1;
    const unsigned char* ptr2_char = (const unsigned char*)ptr2;
    while (num--) {
        if (*ptr1_char != *ptr2_char) {
            return *ptr1_char - *ptr2_char;
        }
        ptr1_char++;
        ptr2_char++;
    }
    return 0;
}