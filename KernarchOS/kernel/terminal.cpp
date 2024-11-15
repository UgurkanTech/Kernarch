#include "terminal.h"
#include "io.h"
#include "commands.h"

volatile uint16_t* const vga_buffer = (uint16_t*)0xB8000;
const int VGA_COLS = 80;
const int VGA_ROWS = 25;
const int PRINTABLE_ROWS = VGA_ROWS - 1;
const int INPUT_BUFFER_SIZE = 256;

// Default VGA colors
const uint8_t DEFAULT_FG = 0x0F;  // White
const uint8_t DEFAULT_BG = 0x00;  // Black
const uint8_t DEFAULT_COLOR = (DEFAULT_BG << 4) | DEFAULT_FG;

// State variables
static int print_col = 0;
static int print_row = 0;
static int input_col = 0;
static uint8_t term_color = DEFAULT_COLOR;  // Initialize with default colors
static char input_buffer[INPUT_BUFFER_SIZE];
static size_t input_index = 0;

Mutex mutex;

// Reset terminal state
static void reset_state() {
    print_col = 0;
    print_row = 0;
    input_col = 0;
    input_index = 0;
    term_color = DEFAULT_COLOR;
}

static inline bool is_within_bounds(int row, int col) {
    return row >= 0 && row < VGA_ROWS && col >= 0 && col < VGA_COLS;
}

static void write_vga(int row, int col, char c, uint8_t color) {
    if (!is_within_bounds(row, col)) return;
    const size_t index = (VGA_COLS * row) + col;
    // Ensure we never write a null character
    if (c == 0) c = ' ';
    vga_buffer[index] = ((uint16_t)color << 8) | (uint8_t)c;
}

static uint16_t read_vga(int row, int col) {
    if (!is_within_bounds(row, col)) return 0;
    const size_t index = (VGA_COLS * row) + col;
    return vga_buffer[index];
}

void update_cursor() {
    const unsigned short position = ((VGA_ROWS - 1) * VGA_COLS) + (input_col % VGA_COLS);
    
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

void term_init() {
    // Reset state first
    reset_state();
    
    // Clear screen with default colors
    for (int row = 0; row < VGA_ROWS; row++) {
        for (int col = 0; col < VGA_COLS; col++) {
            write_vga(row, col, ' ', DEFAULT_COLOR);
        }
    }
    
    // Print initial prompt
    term_printf_at_input_line("> ");
    update_cursor();
}

void term_scroll() {
    // Move all lines up by one
    for (int row = 1; row < PRINTABLE_ROWS; row++) {
        for (int col = 0; col < VGA_COLS; col++) {
            uint16_t character = read_vga(row, col);
            write_vga(row - 1, col, character & 0xFF, (character >> 8) & 0xFF);
        }
    }

    // Clear the last printable line
    for (int col = 0; col < VGA_COLS; col++) {
        write_vga(PRINTABLE_ROWS - 1, col, ' ', term_color);
    }

    print_row = PRINTABLE_ROWS - 1;
    print_col = 0;
}

void term_putc(char c) {
    // Protect against buffer overflows
    if (print_row >= PRINTABLE_ROWS) {
        term_scroll();
    }
    
    if (c == '\n') {
        print_col = 0;
        print_row++;
        if (print_row >= PRINTABLE_ROWS) {
            term_scroll();
        }
        return;
    }
    
    // Only print printable characters
    if (c >= 32 && c <= 126) {
        write_vga(print_row, print_col, c, term_color);
        print_col++;
        
        if (print_col >= VGA_COLS) {
            print_col = 0;
            print_row++;
            if (print_row >= PRINTABLE_ROWS) {
                term_scroll();
            }
        }
    }
}

void term_input_putc(char c) {
    const int INPUT_ROW = VGA_ROWS - 1;
    
    switch (c) {
        case '\n':
            // Clear input line but preserve prompt
            write_vga(INPUT_ROW, 0, '>', term_color);
            write_vga(INPUT_ROW, 1, ' ', term_color);
            for (int col = 2; col < VGA_COLS; col++) {
                write_vga(INPUT_ROW, col, ' ', term_color);
            }
            input_col = 2;  // Reset to after prompt
            break;
            
        case '\b':
            if (input_col > 2) {  // Don't backspace over prompt
                input_col--;
                write_vga(INPUT_ROW, input_col, ' ', term_color);
            }
            break;
            
        default:
            if (input_col < VGA_COLS && c >= 32 && c <= 126) {
                write_vga(INPUT_ROW, input_col, c, term_color);
                input_col++;
            }
            break;
    }
    
    update_cursor();
}

void term_printf_at_input_line(const char* format, ...) {
    if (!format) return;

    char buffer[256];
    va_list args;
    va_start(args, format);
    
    int length = vformat_string(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (length > 0) {
        mutex.lock();
        const int INPUT_ROW = VGA_ROWS - 1;
        for (int i = 0; i < length && i < VGA_COLS; i++) {
            write_vga(INPUT_ROW, i, buffer[i], term_color);
        }
        input_col = length;
        update_cursor();
        mutex.unlock();
    }
    
}

void term_print(const char* str) {
    if (!str) return;
    mutex.lock();
    for (size_t i = 0; str[i] != '\0'; i++) {
        term_putc(str[i]);
    }
    mutex.unlock();
}



void term_printf(const char* format, ...) {
    if (!format) return;

    char buffer[256];
    va_list args;
    va_start(args, format);
    
    int length = vformat_string(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (length > 0) {
        term_print(buffer);
    }
}

//For debugging. Do not use.
void term_unlocked_printf(const char* format, ...) {
    if (!format) return;
    char buffer[256];
    va_list args;
    va_start(args, format);
    
    int length = vformat_string(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (length > 0) {
        for (size_t i = 0; buffer[i] != '\0'; i++) {
            term_putc(buffer[i]);
        }
    }
}

// Function to print to the last location without scrolling
void term_printf_fixed(const char* format, ...) {
    if (!format) return;
    char buffer[256];
    va_list args;
    va_start(args, format);

    int length = vformat_string(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (length > 0) {
        mutex.lock();
        const int INPUT_ROW = VGA_ROWS - 2; // Assuming you want to print on the last row
        for (int i = 0; i < length && i < VGA_COLS; i++) {
            if (buffer[i] != '\n' && buffer[i] != '\0')
                write_vga(INPUT_ROW, i, buffer[i], term_color); // Write to the last row
        }
        mutex.unlock();
    }
}

void term_process_command() {
    mutex.lock();
    // Save terminal state
    const int saved_print_col = print_col;
    const int saved_print_row = print_row;
    const uint8_t saved_color = term_color;
    
    // Restore terminal state
    print_col = saved_print_col;
    print_row = saved_print_row;
    term_color = saved_color;
    
    mutex.unlock();
    // Print new prompt
    term_printf_at_input_line("> ");

    // Execute command
    Commands::execute(input_buffer);
}

void term_input(char c) {
    mutex.lock();
    // Protect against buffer overflow
    if (input_index >= INPUT_BUFFER_SIZE - 1) {
        input_index = 0;  // Reset buffer if it's full
    }
    
    if (c == '\n') {
        input_buffer[input_index] = '\0';
        term_input_putc(c);
        mutex.unlock();
        
        term_process_command();
        mutex.lock();
        input_index = 0;
    } else if (c == '\b') {
        if (input_index > 0) {
            input_index--;
            term_input_putc(c);
        }
    } else if (c >= 32 && c <= 126) {  // Only accept printable characters
        input_buffer[input_index++] = c;
        term_input_putc(c);
    }
   mutex.unlock();
}


void term_clear() {
    mutex.lock();
    for (int row = 0; row < PRINTABLE_ROWS; row++) {
        for (int col = 0; col < VGA_COLS; col++) {
            write_vga(row, col, ' ', term_color);
        }
    }
    print_col = 0;
    print_row = 0;
    mutex.unlock();
}

void set_text_color(uint8_t color) {
    mutex.lock();
    term_color = (term_color & 0xF0) | (color & 0x0F);
    mutex.unlock();
}

void set_text_bg_color(uint8_t color) {
    mutex.lock();
    term_color = (term_color & 0x0F) | ((color & 0x0F) << 4);
    mutex.unlock();
}

//Not thread safe colors..
void term_print_colored(const char* str, vga_color fg_color, vga_color bg_color) {
    if (!str) return;
    
    const uint8_t saved_color = term_color;
    
    term_color = ((bg_color & 0x0F) << 4) | (fg_color & 0x0F);
    
    term_print(str);
    
    term_color = saved_color;
}

uint8_t get_terminal_color() {
    return term_color;
}