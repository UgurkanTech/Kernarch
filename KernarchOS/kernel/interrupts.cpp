#include "interrupts.h"

void handle_software_interrupt(interrupt_frame* frame) {
    switch (frame->int_no) {
        case INT_OUT_OF_MEMORY:
            term_print("Out of memory! Kernel will terminate some processes now.\n");
            break;
        default:
            term_print("Unknown software interrupt!\n");
            break;
    }
}