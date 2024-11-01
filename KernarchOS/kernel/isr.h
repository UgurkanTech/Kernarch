#ifndef ISR_H
#define ISR_H

#include "types.h"
#include "terminal.h"
#include "idt.h"
#include "pic.h"

struct __attribute__((packed)) interrupt_frame {
    uint32_t isr_esp;     // 0: ESP from interrupt_wrapper
    uint32_t gs;          // 4: GS segment
    uint32_t fs;          // 8: FS segment
    uint32_t es;          // 12: ES segment
    uint32_t ds;          // 16: DS segment

    uint32_t edi;         // 20: EDI register
    uint32_t esi;         // 24: ESI register
    uint32_t ebp;         // 28: EBP register
    uint32_t _;           // 32: Unused (ESP saved by pusha but not used)
    uint32_t ebx;         // 36: EBX register
    uint32_t edx;         // 40: EDX register
    uint32_t ecx;         // 44: ECX register
    uint32_t eax;         // 48: EAX register

    uint32_t err_code;    // 52: Error code

    uint32_t eip;         // 56: Instruction pointer
    uint32_t cs;          // 60: Code segment
    uint32_t eflags;      // 64: CPU flags
    uint32_t useresp;     // 68: User stack pointer (if privilege change)
    uint32_t ss;          // 72: Stack segment (if privilege change)
};



extern "C" void (*isr_stub_table[])();

extern "C" void isr_install();
extern "C" void isr_handler(uint8_t vec, interrupt_frame frame);

void print_interrupt_frame(interrupt_frame* frame);

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