#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "isr.h"
#include "pit.h"
#include "types.h"

// Define the system call interrupt number
#define SYSCALL_INT 0x80

// Software Interrupts (custom to kernel)
enum SoftwareInterrupt {
    SYSCALL_PRINT = 1,
    SYSCALL_CLEAR
};

inline void trigger_interrupt(uint8_t interrupt_number) {
    asm volatile ("int %0" : : "N" (interrupt_number));
}

// Function prototype for printf system call
void sys_printf(const char* format, ...);

void syscall_handler(interrupt_frame* frame);

#endif // INTERRUPTS_H