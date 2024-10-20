// string_utils.cpp
#include "string_utils.h"
#include "cstring.h"

void reverse(char* str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

int int_to_string(int num, char* str, int base) {
    int i = 0;
    bool is_negative = false;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return i;
    }

    if (num < 0 && base == 10) {
        is_negative = true;
        num = -num;
    }

    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }

    if (is_negative)
        str[i++] = '-';

    str[i] = '\0';
    reverse(str, i);
    return i;
}

int uint_to_string(unsigned int num, char* str, int base) {
    int i = 0;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return i;
    }

    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }

    str[i] = '\0';
    reverse(str, i);
    return i;
}

int double_to_string(double num, char* str, int precision) {
    if (num < 0) {
        *str++ = '-';
        num = -num;
    }

    int whole = (int)num;
    double fraction = num - whole;

    int i = int_to_string(whole, str, 10);
    if (precision > 0) {
        str[i++] = '.';
        while (precision--) {
            fraction *= 10;
            int digit = (int)fraction;
            str[i++] = digit + '0';
            fraction -= digit;
        }
    }
    str[i] = '\0';
    return i;
}

int vformat_string(char* buffer, size_t buffer_size, const char* format, va_list args) {
    size_t buffer_index = 0;
    const char* format_ptr = format;

    while (*format_ptr != '\0' && buffer_index < buffer_size - 1) {
        if (*format_ptr != '%') {
            buffer[buffer_index++] = *format_ptr++;
            continue;
        }

        format_ptr++;

        switch (*format_ptr) {
            case 'd':
            case 'i': {
                int value = va_arg(args, int);
                char temp[32];
                int len = int_to_string(value, temp, 10);
                for (int i = 0; i < len && buffer_index < buffer_size - 1; i++) {
                    buffer[buffer_index++] = temp[i];
                }
                break;
            }
            case 'u': {
                unsigned int value = va_arg(args, unsigned int);
                char temp[32];
                int len = uint_to_string(value, temp, 10);
                for (int i = 0; i < len && buffer_index < buffer_size - 1; i++) {
                    buffer[buffer_index++] = temp[i];
                }
                break;
            }
            case 'x':
            case 'X': {
                unsigned int value = va_arg(args, unsigned int);
                char temp[32];
                int len = uint_to_string(value, temp, 16);
                for (int i = 0; i < len && buffer_index < buffer_size - 1; i++) {
                    buffer[buffer_index++] = (*format_ptr == 'X') ? toupper(temp[i]) : temp[i];
                }
                break;
            }
            case 'f': {
                double value = va_arg(args, double);
                char temp[32];
                int len = double_to_string(value, temp, 6);  // Default precision of 6
                for (int i = 0; i < len && buffer_index < buffer_size - 1; i++) {
                    buffer[buffer_index++] = temp[i];
                }
                break;
            }
            case 'c': {
                char value = (char)va_arg(args, int);
                buffer[buffer_index++] = value;
                break;
            }
            case 's': {
                const char* value = va_arg(args, const char*);
                while (*value != '\0' && buffer_index < buffer_size - 1) {
                    buffer[buffer_index++] = *value++;
                }
                break;
            }
            case '%':
                buffer[buffer_index++] = '%';
                break;
            default:
                buffer[buffer_index++] = '%';
                if (buffer_index < buffer_size - 1) {
                    buffer[buffer_index++] = *format_ptr;
                }
                break;
        }

        format_ptr++;
    }

    buffer[buffer_index] = '\0';
    return buffer_index;
}

int format_string(char* buffer, size_t buffer_size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vformat_string(buffer, buffer_size, format, args);
    va_end(args);
    return result;
}

char* strchr(const char* s, int c) {
    while (*s != '\0') {
        if (*s == (char)c) {
            return (char*)s;
        }
        s++;
    }
    return 0;
}

char* strrchr(const char* s, int c) {
    const char* last = 0;
    do {
        if (*s == (char)c) {
            last = s;
        }
    } while (*s++);
    return (char*)last;
}

char toupper(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 'A';
    }
    return c;
}

char tolower(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 'a';
    }
    return c;
}