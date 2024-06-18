#include "terminal.h"

volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;
const int VGA_COLS = 80;
const int VGA_ROWS = 25;

int term_col = 0;
int term_row = 0;
uint8_t term_color = 0x0F;

// Textmode
void term_init()
{
	for (int col = 0; col < VGA_COLS; col ++)
	{
		for (int row = 0; row < VGA_ROWS; row ++)
		{
			// Buffer has size (VGA_COLS * VGA_ROWS).
			const size_t index = (VGA_COLS * row) + col;
			// Entries in the VGA buffer take the binary form BBBBFFFFCCCCCCCC, where:
			// - B is the background color
			// - F is the foreground color
			// - C is the ASCII character
			vga_buffer[index] = ((uint16_t)term_color << 8) | ' ';
		}
	}
}

void term_putc(char c)
{

	switch (c)
	{
	case '\n':
		{
			term_col = 0;
			term_row ++;
			break;
		}
		
	default:
		{
			const size_t index = (VGA_COLS * term_row) + term_col;
			vga_buffer[index] = ((uint16_t)term_color << 8) | c;
			term_col ++;
			break;
		}
	}
	
	if (term_col >= VGA_COLS)
	{
		term_col = 0;
		term_row ++;
	}
	
	
	if (term_row >= VGA_ROWS)
	{
		term_col = 0;
		term_row = 0;
	}
}

void term_print(const char* str)
{
	for (size_t i = 0; str[i] != '\0'; i ++)
		term_putc(str[i]);
}

void term_print_int(int value) {
    char buffer[10];  // Assuming we won't have an interrupt number larger than 10 digits
    char* ptr = &buffer[9];  // Start from the end of the buffer
    *ptr = '\0';  // Null-terminate the string
    ptr--;  // Move to the digit place

    if (value == 0) {
        *ptr = '0';  // Handle 0 explicitly
    } else {
        while (value > 0) {
            *ptr = '0' + (value % 10);  // Convert the last digit of value to a character
            value /= 10;  // Remove the last digit from value
            ptr--;  // Move to the next digit place
        }
    }

    // ptr now points to the first digit of the number, so we can print it
    term_print(ptr + 1);
}