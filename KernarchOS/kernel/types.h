// KernarchOS/include/types.h
#ifndef KERNARCHOS_TYPES_H
#define KERNARCHOS_TYPES_H

// Signed integer types
typedef signed char         int8_t;
typedef short               int16_t;
typedef int                 int32_t;
typedef long long           int64_t;

// Unsigned integer types
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

// Size type
typedef unsigned int        size_t;

// Signed size type
typedef int                 ssize_t;

// Pointer integer type
typedef unsigned long       uintptr_t;
typedef long                intptr_t;

// Maximum-width integer types
typedef long long           intmax_t;
typedef unsigned long long  uintmax_t;

// Pointer difference type
typedef long                ptrdiff_t;

// Null pointer constant
#define NULL 0

// Size type
#ifndef __SIZE_TYPE__
#define __SIZE_TYPE__ unsigned int
#endif
typedef __SIZE_TYPE__       size_t;

#endif // KERNARCHOS_TYPES_H