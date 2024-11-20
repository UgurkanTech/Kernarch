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
    SYSCALL_READ,
    SYSCALL_CLEAR,
    SYSCALL_EXIT
};

// Function prototype for printf system call
void sys_printf(const char* format, ...);
char sys_read();
void sys_clear();

void syscall_handler(interrupt_frame* frame);

#endif // INTERRUPTS_H