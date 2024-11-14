#include "keyboard.h"
#include "io.h"
#include "terminal.h"
#include "pic.h"

char Keyboard::buffer[KEYBOARD_BUFFER_SIZE];
int Keyboard::buffer_start = 0;
int Keyboard::buffer_end = 0;

const char Keyboard::scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

void Keyboard::init() {
    register_interrupt_handler(INT_KEYBOARD, handle_interrupt);
}

void Keyboard::handle_interrupt(interrupt_frame* frame) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
     // Check if it's a key press (ignore key release)
    if (!(scancode & 0x80)) {
        if (scancode < sizeof(scancode_to_ascii)) {
            char ascii = scancode_to_ascii[scancode];
            if (ascii != 0) {
                enqueue_char(ascii);
            }
        }
    }
}

char Keyboard::get_char() {
    while (!has_char()) {
        // Wait for a character
        asm volatile("nop");
    }
    return dequeue_char();
}

bool Keyboard::has_char() {
    return buffer_start != buffer_end;
}

void Keyboard::enqueue_char(char c) {
    int next_end = (buffer_end + 1) % KEYBOARD_BUFFER_SIZE;
    if (next_end != buffer_start) {
        buffer[buffer_end] = c;
        buffer_end = next_end;
    }
    // If buffer is full, the character is discarded
}

char Keyboard::dequeue_char() {
    if (buffer_start != buffer_end) {
        char c = buffer[buffer_start];
        buffer_start = (buffer_start + 1) % KEYBOARD_BUFFER_SIZE;
        return c;
    }
    return 0; // Return null character if buffer is empty
}