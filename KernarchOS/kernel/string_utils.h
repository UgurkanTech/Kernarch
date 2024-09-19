#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>
#include <stdint.h>

void int_to_string(int value, char* buffer, size_t buffer_size);
void int_to_string(int value, char* str, int base);
void uint_to_string(unsigned int value, char* buffer, size_t buffer_size);
void hex_to_string(unsigned int value, char* buffer, size_t buffer_size);
void pointer_to_string(uintptr_t value, char* buffer, size_t buffer_size);

template<typename... Args>
void format_string(char* buffer, size_t buffer_size, const char* format, Args... args);

// Forward declare the implementation functions
template<typename T, typename... Args>
void format_string_impl(char* buffer, size_t buffer_size, const char* format, T value, Args... args);

void format_string_impl(char* buffer, size_t buffer_size, const char* format);

// Declare the overloaded format_arg functions
void format_arg(char*& buffer, size_t& buffer_size, const char*& format, int value);
void format_arg(char*& buffer, size_t& buffer_size, const char*& format, unsigned int value);
void format_arg(char*& buffer, size_t& buffer_size, const char*& format, const char* value);
void format_arg(char*& buffer, size_t& buffer_size, const char*& format, void* value);

extern template void format_string<char*>(char*, size_t, const char*, char*);
extern template void format_string<int, const char*, unsigned int, unsigned int, const char*, unsigned int>(char*, size_t, const char*, int, const char*, unsigned int, unsigned int, const char*, unsigned int);
extern template void format_string<int, const char*, unsigned int, const char*, unsigned int>(char*, size_t, const char*, int, const char*, unsigned int, const char*, unsigned int);
extern template void format_string<const char*, unsigned int>(char*, size_t, const char*, const char*, unsigned int);


void format_string(char* buffer, unsigned int buffer_size, const char* format, unsigned int value);
void format_string(char* buffer, unsigned int buffer_size, const char* format, int value);
void format_string(char* buffer, unsigned int buffer_size, const char* format, unsigned char value);
void format_string(char* buffer, unsigned int buffer_size, const char* format, const char* value);

#endif // STRING_UTILS_H