#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "isr.h"
#include "pit.h"
#include "types.h"

// Define the system call interrupt number
#define SYSCALL_INT 0x80

// Software Interrupts (custom to kernel)
enum SoftwareInterrupt {
    SYSCALL_SCHEDULE = 1,
    SYSCALL_PRINT,
    SYSCALL_READ,
    SYSCALL_CLEAR,
    SYSCALL_EXIT,
    SYSCALL_SLEEP,
    SYSCALL_TEST
};

// Function prototype for printf system call
void sys_schedule();
void sys_printf(const char* format, ...);
char sys_read();
void sys_clear();
void sys_sleep(uint32_t milliseconds);
void sys_test();

void testThread(const char* name);

void syscall_handler(interrupt_frame* frame);

#endif // INTERRUPTS_H