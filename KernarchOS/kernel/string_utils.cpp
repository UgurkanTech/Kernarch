#include "string_utils.h"

void int_to_string(int value, char* buffer, size_t buffer_size) {
    if (buffer_size == 0) return;

    // Handle negative numbers
    bool is_negative = false;
    if (value < 0) {
        is_negative = true;
        value = -value;
    }

    // Convert integer to string (reversed)
    size_t i = 0;
    do {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0 && i < buffer_size - 1);

    // Add negative sign if necessary
    if (is_negative && i < buffer_size - 1) {
        buffer[i++] = '-';
    }

    // Null-terminate the string
    buffer[i] = '\0';

    // Reverse the string
    for (size_t j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}

void int_to_string(int value, char* str, int base = 10) {
    static char digits[] = "0123456789ABCDEF";
    char* p = str;
    char* p1, *p2;
    unsigned long ud = value;
    int divisor = 10;

    if (base == 16)
        divisor = 16;

    do {
        *p++ = digits[ud % divisor];
    } while (ud /= divisor);

    *p = 0;

    p1 = str;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}

void uint_to_string(unsigned int value, char* buffer, size_t buffer_size) {
    if (buffer_size == 0) return;

    // Convert unsigned integer to string (reversed)
    size_t i = 0;
    do {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0 && i < buffer_size - 1);

    // Null-terminate the string
    buffer[i] = '\0';

    // Reverse the string
    for (size_t j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}

void hex_to_string(unsigned int value, char* buffer, size_t buffer_size) {
    if (buffer_size == 0) return;

    const char* hex_digits = "0123456789ABCDEF";
    
    // Convert unsigned integer to hexadecimal string (reversed)
    size_t i = 0;
    do {
        buffer[i++] = hex_digits[value & 0xF];
        value >>= 4;
    } while (value > 0 && i < buffer_size - 1);

    // Add '0x' prefix
    if (i < buffer_size - 2) {
        buffer[i++] = 'x';
        buffer[i++] = '0';
    }

    // Null-terminate the string
    buffer[i] = '\0';

    // Reverse the string
    for (size_t j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}

void pointer_to_string(uintptr_t value, char* buffer, size_t buffer_size) {
    if (buffer_size == 0) return;

    const char* hex_digits = "0123456789ABCDEF";
    
    // Convert pointer to hexadecimal string (reversed)
    size_t i = 0;
    do {
        buffer[i++] = hex_digits[value & 0xF];
        value >>= 4;
    } while (value > 0 && i < buffer_size - 1);

    // Add '0x' prefix
    if (i < buffer_size - 2) {
        buffer[i++] = 'x';
        buffer[i++] = '0';
    }

    // Null-terminate the string
    buffer[i] = '\0';

    // Reverse the string
    for (size_t j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}


void format_arg(char*& buffer, size_t& buffer_size, const char*& format, int value) {
    switch (*format) {
        case 'd':
            int_to_string(value, buffer, buffer_size);
            break;
        case 'x':
        case 'X':
            hex_to_string((unsigned int)value, buffer, buffer_size);
            break;
        default:
            if (buffer_size > 1) {
                *buffer++ = '%';
                buffer_size--;
            }
            if (buffer_size > 1) {
                *buffer++ = *format;
                buffer_size--;
            }
            break;
    }
    while (*buffer && buffer_size > 0) {
        buffer++;
        buffer_size--;
    }
}

void format_arg(char*& buffer, size_t& buffer_size, const char*& format, unsigned int value) {
    switch (*format) {
        case 'u':
            uint_to_string(value, buffer, buffer_size);
            break;
        case 'x':
        case 'X':
            hex_to_string(value, buffer, buffer_size);
            break;
        default:
            if (buffer_size > 1) {
                *buffer++ = '%';
                buffer_size--;
            }
            if (buffer_size > 1) {
                *buffer++ = *format;
                buffer_size--;
            }
            break;
    }
    while (*buffer && buffer_size > 0) {
        buffer++;
        buffer_size--;
    }
}

void format_arg(char*& buffer, size_t& buffer_size, const char*& format, const char* value) {
    if (*format == 's') {
        while (*value && buffer_size > 1) {
            *buffer++ = *value++;
            buffer_size--;
        }
        *buffer = '\0';
    } else {
        if (buffer_size > 1) {
            *buffer++ = '%';
            buffer_size--;
        }
        if (buffer_size > 1) {
            *buffer++ = *format;
            buffer_size--;
        }
    }
}

void format_arg(char*& buffer, size_t& buffer_size, const char*& format, void* value) {
    if (*format == 'p') {
        pointer_to_string((uintptr_t)value, buffer, buffer_size);
    } else {
        if (buffer_size > 1) {
            *buffer++ = '%';
            buffer_size--;
        }
        if (buffer_size > 1) {
            *buffer++ = *format;
            buffer_size--;
        }
    }
}

template<typename T, typename... Args>
void format_string_impl(char* buffer, size_t buffer_size, const char* format, T value, Args... args) {
    while (*format && buffer_size > 1) {
        if (*format == '%') {
            format++;
            format_arg(buffer, buffer_size, format, value);
            format++;
            format_string_impl(buffer, buffer_size, format, args...);
            return;
        } else {
            *buffer++ = *format++;
            buffer_size--;
        }
    }
    *buffer = '\0';
}

void format_string_impl(char* buffer, size_t buffer_size, const char* format) {
    while (*format && buffer_size > 1) {
        *buffer++ = *format++;
        buffer_size--;
    }
    *buffer = '\0';
}

template<typename... Args>
void format_string(char* buffer, size_t buffer_size, const char* format, Args... args) {
    format_string_impl(buffer, buffer_size, format, args...);
}

void format_string(char* buffer, unsigned int buffer_size, const char* format, unsigned int value) {
    char int_buffer[32];
    int_to_string(value, int_buffer);
    
    while (*format && buffer_size > 1) {
        if (*format == '%' && *(format + 1) == 'u') {
            const char* p = int_buffer;
            while (*p && buffer_size > 1) {
                *buffer++ = *p++;
                buffer_size--;
            }
            format += 2;
        } else {
            *buffer++ = *format++;
            buffer_size--;
        }
    }
    *buffer = '\0';
}

void format_string(char* buffer, unsigned int buffer_size, const char* format, int value) {
    char int_buffer[32];
    int_to_string(value, int_buffer);
    
    while (*format && buffer_size > 1) {
        if (*format == '%' && *(format + 1) == 'd') {
            const char* p = int_buffer;
            while (*p && buffer_size > 1) {
                *buffer++ = *p++;
                buffer_size--;
            }
            format += 2;
        } else {
            *buffer++ = *format++;
            buffer_size--;
        }
    }
    *buffer = '\0';
}

void format_string(char* buffer, unsigned int buffer_size, const char* format, unsigned char value) {
    char int_buffer[32];
    int_to_string(value, int_buffer);
    
    while (*format && buffer_size > 1) {
        if (*format == '%' && *(format + 1) == 'c') {
            *buffer++ = value;
            buffer_size--;
            format += 2;
        } else if (*format == '%' && *(format + 1) == 'u') {
            const char* p = int_buffer;
            while (*p && buffer_size > 1) {
                *buffer++ = *p++;
                buffer_size--;
            }
            format += 2;
        } else {
            *buffer++ = *format++;
            buffer_size--;
        }
    }
    *buffer = '\0';
}

void format_string(char* buffer, unsigned int buffer_size, const char* format, const char* value) {
    while (*format && buffer_size > 1) {
        if (*format == '%' && *(format + 1) == 's') {
            const char* p = value;
            while (*p && buffer_size > 1) {
                *buffer++ = *p++;
                buffer_size--;
            }
            format += 2;
        } else {
            *buffer++ = *format++;
            buffer_size--;
        }
    }
    *buffer = '\0';
}

// Explicit instantiations for common types
template void format_string(char*, size_t, const char*);
template void format_string(char*, size_t, const char*, int);
template void format_string(char*, size_t, const char*, unsigned int);
template void format_string(char*, size_t, const char*, const char*);
template void format_string(char*, size_t, const char*, void*);
template void format_string(char*, size_t, const char*, int, const char*);
template void format_string(char*, size_t, const char*, const char*, int);
template void format_string(char*, size_t, const char*, int, int);
template void format_string(char*, size_t, const char*, const char*, const char*);

template void format_string<char*>(char*, size_t, const char*, char*);
template void format_string<int, const char*, unsigned int, unsigned int, const char*, unsigned int>(char*, size_t, const char*, int, const char*, unsigned int, unsigned int, const char*, unsigned int);
template void format_string<int, const char*, unsigned int, const char*, unsigned int>(char*, size_t, const char*, int, const char*, unsigned int, const char*, unsigned int);
template void format_string<const char*, unsigned int>(char*, size_t, const char*, const char*, unsigned int);