#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "isr.h"
#include <stdint.h>

// Software Interrupts (custom to kernel)
enum SoftwareInterrupt {
    INT_OUT_OF_MEMORY = 48,
};

void handle_software_interrupt(interrupt_frame* frame);



inline void trigger_interrupt(uint8_t interrupt_number) {
    asm volatile ("int %0" : : "N" (interrupt_number));
}

#endif // INTERRUPTS_H