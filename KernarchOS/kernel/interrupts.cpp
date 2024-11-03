#include "interrupts.h"
#include "process.h"

void handle_software_interrupt(uint8_t int_no, interrupt_frame* frame) {
    switch (int_no) {
        case INT_OUT_OF_MEMORY:
            term_print("Out of memory! Kernel will terminate some processes now.\n");
            break;
        case Yield:
            term_print("Yield interrupt triggered!\n");
            //schedule();
            break;
        case Sleep:
            for (int i = 0; i < 35000000; i++)
                asm volatile ("nop");
            break;
        default:
            term_print("Unknown software interrupt!\n");
            break;
    }
}