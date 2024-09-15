#include "string_utils.h"
#include <stdarg.h>

void int_to_string(int value, char* str) {
    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    int i = 0;
    bool is_negative = false;
    if (value < 0) {
        is_negative = true;
        value = -value;
    }

    while (value != 0) {
        str[i++] = (value % 10) + '0';
        value /= 10;
    }

    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void uint_to_string(unsigned int value, char* str) {
    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    int i = 0;
    while (value != 0) {
        str[i++] = (value % 10) + '0';
        value /= 10;
    }

    str[i] = '\0';

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void format_string(char* buffer, int buffer_size, const char* format, ...) {
    va_list args;
    va_start(args, format);

    int buffer_index = 0;
    const char* format_ptr = format;

    while (*format_ptr != '\0' && buffer_index < buffer_size - 1) {
        if (*format_ptr == '%') {
            format_ptr++;
            switch (*format_ptr) {
                case 'd': {
                    int value = va_arg(args, int);
                    char int_str[12];
                    int_to_string(value, int_str);
                    for (int i = 0; int_str[i] != '\0' && buffer_index < buffer_size - 1; i++) {
                        buffer[buffer_index++] = int_str[i];
                    }
                    break;
                }
                case 'u': {
                    unsigned int value = va_arg(args, unsigned int);
                    char uint_str[11];
                    uint_to_string(value, uint_str);
                    for (int i = 0; uint_str[i] != '\0' && buffer_index < buffer_size - 1; i++) {
                        buffer[buffer_index++] = uint_str[i];
                    }
                    break;
                }
                case 's': {
                    const char* str = va_arg(args, const char*);
                    while (*str != '\0' && buffer_index < buffer_size - 1) {
                        buffer[buffer_index++] = *str++;
                    }
                    break;
                }
                case 'x': {
                    unsigned int value = va_arg(args, unsigned int);
                    char hex_str[9];
                    for (int i = 7; i >= 0; i--) {
                        int digit = value & 0xF;
                        hex_str[i] = digit < 10 ? '0' + digit : 'A' + digit - 10;
                        value >>= 4;
                    }
                    hex_str[8] = '\0';
                    for (int i = 0; hex_str[i] != '\0' && buffer_index < buffer_size - 1; i++) {
                        buffer[buffer_index++] = hex_str[i];
                    }
                    break;
                }
                default:
                    buffer[buffer_index++] = *format_ptr;
                    break;
            }
        } else {
            buffer[buffer_index++] = *format_ptr;
        }
        format_ptr++;
    }

    buffer[buffer_index] = '\0';
    va_end(args);
}