#include "terminal.h"
#include "io.h"

volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;
const int VGA_COLS = 80;
const int VGA_ROWS = 25;

int term_col = 0;
int term_row = 0;
uint8_t term_color = 0x0F;


void update_cursor()
{
    unsigned short position = (term_row * VGA_COLS) + term_col;

    // cursor LOW port to vga INDEX register
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));
    // cursor HIGH port to vga INDEX register
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}


// Textmode
void term_init()
{
    for (int col = 0; col < VGA_COLS; col ++)
    {
        for (int row = 0; row < VGA_ROWS; row ++)
        {
            const size_t index = (VGA_COLS * row) + col;
            vga_buffer[index] = ((uint16_t)term_color << 8) | ' ';
        }
    }
    term_col = 0;
    term_row = 0;
    update_cursor();
}

void term_scroll()
{
    // Move all lines up by one
    for (int row = 1; row < VGA_ROWS; row++)
    {
        for (int col = 0; col < VGA_COLS; col++)
        {
            const size_t to_index = (VGA_COLS * (row - 1)) + col;
            const size_t from_index = (VGA_COLS * row) + col;
            vga_buffer[to_index] = vga_buffer[from_index];
        }
    }

    // Clear the last line
    for (int col = 0; col < VGA_COLS; col++)
    {
        const size_t index = (VGA_COLS * (VGA_ROWS - 1)) + col;
        vga_buffer[index] = ((uint16_t)term_color << 8) | ' ';
    }

    // Move cursor to the start of the last line
    term_row = VGA_ROWS - 1;
    term_col = 0;
    update_cursor();
}

void term_putc(char c)
{
    switch (c)
    {
    case '\n':
        {
            term_col = 0;
            term_row++;
            break;
        }
        
    default:
        {
            const size_t index = (VGA_COLS * term_row) + term_col;
            vga_buffer[index] = ((uint16_t)term_color << 8) | c;
            term_col++;
            break;
        }
    }
    
    if (term_col >= VGA_COLS)
    {
        term_col = 0;
        term_row++;
    }
    
    if (term_row >= VGA_ROWS)
    {
        term_scroll();
    }
    update_cursor();
}

void term_print(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i++)
        term_putc(str[i]);
}

void term_print_int(int n) {
    if (n == 0) {
        term_print("0");
        return;
    }

    if (n < 0) {
        term_putc('-');
        n = -n;
    }

    char buffer[20];
    int i = 0;
    while (n > 0) {
        buffer[i++] = '0' + (n % 10);
        n /= 10;
    }

    while (--i >= 0) {
        term_putc(buffer[i]);
    }
}

void term_print_uint(unsigned int n) {
    if (n == 0) {
        term_print("0");
        return;
    }

    char buffer[20];
    int i = 0;
    while (n > 0) {
        buffer[i++] = '0' + (n % 10);
        n /= 10;
    }

    while (--i >= 0) {
        term_putc(buffer[i]);
    }
}

void term_print_hex(uint32_t value) {
    const char* hex_chars = "0123456789ABCDEF";
    char buffer[9];  // 8 hex digits + null terminator
    buffer[8] = '\0';

    for (int i = 7; i >= 0; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }

    term_print(buffer);
}

#include "commands.h"

#define INPUT_BUFFER_SIZE 256

char input_buffer[INPUT_BUFFER_SIZE];
size_t input_index = 0;

void term_input(char c) {
    if (c == '\n') {
        term_putc(c);
        input_buffer[input_index] = '\0';
        term_process_command();
        input_index = 0;
    } else if (c == '\b') {
        if (input_index > 0) {
            input_index--;
            term_backspace();
        }
    } else if (input_index < INPUT_BUFFER_SIZE - 1) {
        input_buffer[input_index++] = c;
        term_putc(c);
    }
}

void term_process_command() {
    Commands::execute(input_buffer);
    term_print("> ");
}

void term_backspace() {
    if (term_col > 0) {
        term_col--;
        size_t index = (VGA_COLS * term_row) + term_col;
        vga_buffer[index] = ((uint16_t)term_color << 8) | ' ';
    }
    update_cursor();
}

void term_clear() {
    for (int row = 0; row < VGA_ROWS; row++) {
        for (int col = 0; col < VGA_COLS; col++) {
            size_t index = (VGA_COLS * row) + col;
            vga_buffer[index] = ((uint16_t)term_color << 8) | ' ';
        }
    }
    term_col = 0;
    term_row = 0;
    update_cursor();
}

void term_print_colored(const char* str, vga_color fg_color, vga_color bg_color) {
    uint8_t old_color = get_terminal_color();
    set_text_color(fg_color);
    set_text_bg_color(bg_color);
    term_print(str);
    term_color = old_color;
}


void set_text_color(uint8_t color)
{
    term_color = (term_color & 0xF0) | (color & 0x0F);
}

void set_text_bg_color(uint8_t color)
{
    term_color = (term_color & 0x0F) | ((color & 0x0F) << 4);
}

uint8_t get_terminal_color()
{
    return term_color;
}

