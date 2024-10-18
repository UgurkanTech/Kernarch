#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
#include "isr.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_BUFFER_SIZE 256

class Keyboard {
public:
    static void init();
    static void handle_interrupt(interrupt_frame* frame);
    static char get_char();
    static bool has_char();

private:
    static char buffer[KEYBOARD_BUFFER_SIZE];
    static int buffer_start;
    static int buffer_end;
    static const char scancode_to_ascii[];

    static void enqueue_char(char c);
    static char dequeue_char();
};

#endif // KEYBOARD_H