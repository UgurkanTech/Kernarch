// string_utils.h
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include "cstring.h"

// Define va_list if not already defined
#ifndef _VA_LIST
#define _VA_LIST
typedef __builtin_va_list va_list;
#endif

#define va_start(v,l) __builtin_va_start(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v,l) __builtin_va_arg(v,l)

// Function to format a string
int format_string(char* buffer, size_t buffer_size, const char* format, ...);

// Helper function to format a string with va_list
int vformat_string(char* buffer, size_t buffer_size, const char* format, va_list args);

// Helper functions
void reverse(char* str, int length);
int int_to_string(int num, char* str, int base);
int uint_to_string(unsigned int num, char* str, int base);
int double_to_string(double num, char* str, int precision);

// Additional string utility functions not present in cstring.h
char* strchr(const char* s, int c);
char* strrchr(const char* s, int c);
char toupper(char c);
char tolower(char c);

#endif // STRING_UTILS_H