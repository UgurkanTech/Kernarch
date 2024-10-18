#ifndef KERNARCH_CSTRING_H
#define KERNARCH_CSTRING_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);
void* memmove(void* dest, const void* src, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);

int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
size_t strlen(const char* s);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
char* strcat(char* dest, const char* src);
char* strncat(char* dest, const char* src, size_t n);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace std {
    using ::memcpy;
    using ::memset;
    using ::memmove;
    using ::memcmp;
    using ::strcmp;
    using ::strncmp;
    using ::strlen;
    using ::strcpy;
    using ::strncpy;
    using ::strcat;
    using ::strncat;
}
#endif

#endif // KERNARCH_CSTRING_H