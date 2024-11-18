#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"
#include "string_utils.h"
#include "mutex.h"

enum vga_color {
    VGA_BLACK = 0x0,
    VGA_BLUE = 0x1,
    VGA_GREEN = 0x2,
    VGA_CYAN = 0x3,
    VGA_RED = 0x4,
    VGA_MAGENTA = 0x5,
    VGA_BROWN = 0x6,
    VGA_LIGHT_GRAY = 0x7,
    VGA_DARK_GRAY = 0x8,
    VGA_LIGHT_BLUE = 0x9,
    VGA_LIGHT_GREEN = 0xa,
    VGA_LIGHT_CYAN = 0xb,
    VGA_LIGHT_RED = 0xc,
    VGA_LIGHT_MAGENTA = 0xd,
    VGA_YELLOW = 0xe,
    VGA_WHITE = 0xf,
};

void term_init();

void term_print(const char* str);

void term_printf(const char* str, ...);
void term_printf_fixed(const char* format, ...);

void term_input(char c);
void term_printf_at_input_line(const char* format, ...);

void term_clear();

void term_unlocked_printf(const char* format, ...);

#endif // TERMINAL_H