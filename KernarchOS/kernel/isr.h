#ifndef ISR_H
#define ISR_H

#include "types.h"
#include "terminal.h"
#include "idt.h"
#include "pic.h"
#include "logger.h"

typedef struct __attribute__((packed, aligned(4))) interrupt_frame{
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
    uint32_t esp;         // 68: User stack pointer (if privilege change)
    uint32_t ss;          // 72: Stack segment (if privilege change)
} interruptFrame;



extern "C" void (*isr_stub_table[])();

extern "C" void isr_install();
extern "C" void isr_handler(uint8_t vec, interrupt_frame frame);

#define MAX_HANDLERS_PER_INTERRUPT 8
#define EXCEPTION_BASE 0
#define HARDWARE_INT_BASE 32
#define SOFTWARE_INT_BASE 48

typedef enum {
    INTERRUPT_TYPE_EXCEPTION,
    INTERRUPT_TYPE_HARDWARE,
    INTERRUPT_TYPE_SOFTWARE,
    INTERRUPT_TYPE_UNKNOWN
} interrupt_type_t;

typedef void (*interrupt_handler_t)(interrupt_frame* frame);

bool register_interrupt_handler(uint8_t vector, interrupt_handler_t handler);
bool unregister_interrupt_handler(uint8_t vector, interrupt_handler_t handler);

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

constexpr const char* exception_messages[] = {
    "Division By Zero", "Debug", "Non Maskable Interrupt", "Breakpoint",
    "Into Detected Overflow", "Out of Bounds", "Invalid Opcode", "No Coprocessor",
    "Double Fault", "Coprocessor Segment Overrun", "Bad TSS", "Segment Not Present",
    "Stack Fault", "General Protection Fault", "Page Fault", "Unknown Interrupt",
    "Coprocessor Fault", "Alignment Check", "Machine Check", "SIMD Floating-Point Exception",
    "Virtualization Exception", "Control Protection Exception"
};

inline static uint32_t read_cr2() { uint32_t cr2; asm volatile("mov %%cr2, %0" : "=r" (cr2)); return cr2; }

inline static void print_interrupt_frame(interrupt_frame* ctx, uint8_t vec = 0xFF) {
    if (vec != 0xFF) {
        if (vec < sizeof(exception_messages) / sizeof(char*)) {
            Logger::serial_log("Exception: %s\n", exception_messages[vec]);
        } else {
            Logger::serial_log("Unknown Exception (Interrupt Number: %u)\n", vec);
        }
    }

    Logger::serial_log("Interrupt Frame:\n"
        "EIP:     0x%x\n"
        "CS:      0x%x\n"
        "EFLAGS:  0x%x\n"
        "ESP:     0x%x\n"
        "SS:      0x%x\n"
        "EAX:     0x%x\n"
        "EBX:     0x%x\n"
        "ECX:     0x%x\n"
        "EDX:     0x%x\n"
        "ESI:     0x%x\n"
        "EDI:     0x%x\n"
        "EBP:     0x%x\n"
        "GS:      0x%x\n"
        "FS:      0x%x\n"
        "ES:      0x%x\n"
        "DS:      0x%x\n",
        ctx->eip, ctx->cs, ctx->eflags, ctx->esp, ctx->ss,
        ctx->eax, ctx->ebx, ctx->ecx, ctx->edx,
        ctx->esi, ctx->edi, ctx->ebp,
        ctx->gs, ctx->fs, ctx->es, ctx->ds
    );

    if (ctx->err_code != 0xFFFFFFFF) {
        Logger::serial_log("Error Code: 0x%x\n", ctx->err_code);
        if (vec == 14) { // Page Fault
            Logger::serial_log("Page Fault Details: ");
            if (ctx->err_code & 0x1) Logger::serial_log("Page-Protection Violation ");
            else Logger::serial_log("Non-present Page ");
            if (ctx->err_code & 0x2) Logger::serial_log("Write ");
            else Logger::serial_log("Read ");
            if (ctx->err_code & 0x4) Logger::serial_log("User Mode ");
            else Logger::serial_log("Kernel Mode ");
            if (ctx->err_code & 0x8) Logger::serial_log("Reserved Bit Overwrite ");
            if (ctx->err_code & 0x10) Logger::serial_log("Instruction Fetch ");
            Logger::serial_log("\nFaulting Address: 0x%x\n", read_cr2());
        }
    }
}

#endif // ISR_H