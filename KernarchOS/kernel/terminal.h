#ifndef TERMINAL_H
#define TERMINAL_H

#include <stddef.h>
#include <stdint.h>

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

#endif // TERMINAL_H