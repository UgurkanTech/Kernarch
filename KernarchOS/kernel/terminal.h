#ifndef TERMINAL_H
#define TERMINAL_H

#include <stddef.h>
#include <stdint.h>

enum vga_color {
    VGA_BLACK = 0,
    VGA_BLUE = 1,
    VGA_GREEN = 2,
    VGA_CYAN = 3,
    VGA_RED = 4,
    VGA_MAGENTA = 5,
    VGA_BROWN = 6,
    VGA_LIGHT_GRAY = 7,
    VGA_DARK_GRAY = 8,
    VGA_LIGHT_BLUE = 9,
    VGA_LIGHT_GREEN = 10,
    VGA_LIGHT_CYAN = 11,
    VGA_LIGHT_RED = 12,
    VGA_LIGHT_MAGENTA = 13,
    VGA_YELLOW = 14,
    VGA_WHITE = 15,
};

void term_init();
void term_putc(char c);
void term_print(const char* str);
void term_print_int(int value);
void term_print_uint(unsigned int n);
void term_print_hex(uint32_t value);
void term_backspace();
void term_clear();
void term_input(char c);
void term_process_command();
void set_text_color(uint8_t color);
void set_text_bg_color(uint8_t color);

void term_print_colored(const char* str, vga_color fg_color, vga_color bg_color = VGA_BLACK);

uint8_t get_terminal_color();

#endif // TERMINAL_H