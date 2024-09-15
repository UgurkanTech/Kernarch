#ifndef ISR_H
#define ISR_H

#include <stdint.h>
#include "terminal.h"
#include "idt.h"
#include "pic.h"

struct interrupt_frame {
    uint32_t ds;                                     // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha
    uint32_t int_no, err_code;                       // Interrupt number and error code
    uint32_t eip, cs, eflags, useresp, ss;           // Pushed by the processor automatically
};

extern "C" void (*isr_stub_table[])();

extern "C" void isr_install();
extern "C" void isr_handler(interrupt_frame* frame);



// CPU Exceptions
enum CPUException {
    EXC_DIVIDE_ERROR = 0,
    EXC_DEBUG,
    EXC_NMI,
    EXC_BREAKPOINT,
    EXC_OVERFLOW,
    EXC_BOUND_RANGE_EXCEEDED,
    EXC_INVALID_OPCODE,
    EXC_DEVICE_NOT_AVAILABLE,
    EXC_DOUBLE_FAULT,
    EXC_COPROCESSOR_SEGMENT_OVERRUN,
    EXC_INVALID_TSS,
    EXC_SEGMENT_NOT_PRESENT,
    EXC_STACK_SEGMENT_FAULT,
    EXC_GENERAL_PROTECTION,
    EXC_PAGE_FAULT,
    EXC_RESERVED,
    EXC_X87_FPU_ERROR,
    EXC_ALIGNMENT_CHECK,
    EXC_MACHINE_CHECK,
    EXC_SIMD_FP_EXCEPTION,
    EXC_VIRTUALIZATION,
    EXC_CONTROL_PROTECTION,
    // 22-31 are reserved
};

// Hardware Interrupts
enum HardwareInterrupt {
    INT_TIMER = 32,
    INT_KEYBOARD,
    INT_CASCADE,
    INT_COM2,
    INT_COM1,
    INT_LPT2,
    INT_FLOPPY,
    INT_LPT1,
    INT_RTC,
    INT_FREE1,
    INT_FREE2,
    INT_FREE3,
    INT_MOUSE,
    INT_FPU,
    INT_PRIMARY_ATA,
    INT_SECONDARY_ATA
};

#endif // ISR_H