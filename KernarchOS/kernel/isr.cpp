#include "isr.h"
#include "idt.h"
#include "pic.h"
#include "pit.h"
#include "terminal.h"
#include "keyboard.h"
#include "interrupts.h"
#include "acpi.h"
#include "kernel_config.h"

const char* exception_messages[] = {
    "Division By Zero", "Debug", "Non Maskable Interrupt", "Breakpoint",
    "Into Detected Overflow", "Out of Bounds", "Invalid Opcode", "No Coprocessor",
    "Double Fault", "Coprocessor Segment Overrun", "Bad TSS", "Segment Not Present",
    "Stack Fault", "General Protection Fault", "Page Fault", "Unknown Interrupt",
    "Coprocessor Fault", "Alignment Check", "Machine Check", "SIMD Floating-Point Exception",
    "Virtualization Exception", "Control Protection Exception"
};

void page_fault_handler(interrupt_frame* frame) {
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    term_print("FATAL: Page Fault!\n");
    term_print("Faulting address: 0x");
    term_print_hex(faulting_address);
    term_print("\nError code: 0x");
    term_print_hex(frame->err_code);
    term_print("\nAt EIP: 0x");
    term_print_hex(frame->eip);
    term_print(" CS: 0x");
    term_print_hex(frame->cs);
    term_print(" SS: 0x");
    term_print_hex(frame->ss);
    term_print(" userESP: 0x");
    term_print_hex(frame->useresp);

    term_print("\n");

    if (faulting_address >= (uint32_t)&stack_guard_bottom && faulting_address < (uint32_t)&stack_top) {
        term_print("Stack overflow detected!\n");
    } else {
        term_print("General Page Fault ( ");
        if (!(frame->err_code & 0x1)) term_print("Page not present ");
        if (frame->err_code & 0x2) term_print("Write operation");
        if (frame->err_code & 0x4) term_print("User-mode ");
        if (frame->err_code & 0x8) term_print("Reserved bits overwritten");
        if (frame->err_code & 0x10) term_print("Instruction fetch ");
        term_print(")\n");
    }

    term_print("System halted.\n");

    // Disable interrupts and halt the CPU
    //asm volatile("cli; hlt");

    // In case an NMI wakes the CPU, loop forever
    for(;;);
}

extern "C" void isr_handler(interrupt_frame* frame) {
    if (frame->int_no < 32) {
        // Handle exceptions
        switch(frame->int_no) {
            case EXC_PAGE_FAULT:
                page_fault_handler(frame);
                break;
            case EXC_DIVIDE_ERROR:
                term_print("Division by zero exception\n");
                frame->eip += 2; // Skip the instruction that caused the exception XD
                break;
            default:
                term_print("Unhandled CPU Exception: ");
                term_print_uint(frame->int_no);
                term_print(" - ");
                term_print(exception_messages[frame->int_no]);
                term_print("\n");
                break;
        }

        pic_sendEOI(frame->int_no);
    } else if (frame->int_no < 48) {
        // Handle hardware interrupts
        uint8_t irq = frame->int_no - 32;
        switch (frame->int_no) {
            case INT_TIMER:
                pit_handler();
                break;
            case INT_KEYBOARD:
                Keyboard::handle_interrupt(frame);
                break;
            case INT_PRIMARY_ATA:
                ACPI::instance()->handle_primary_ide_interrupt();
                break;
            case INT_SECONDARY_ATA:
                ACPI::instance()->handle_secondary_ide_interrupt();
                break;
            default:
                term_print("Unhandled hardware interrupt: ");
                term_print_uint(irq);
                term_print("\n");
                break;
        }
        pic_sendEOI(irq);
    } else if (frame->int_no < 256) {
        // Handle software interrupts
        handle_software_interrupt(frame);
    } else {
        term_print("Received unexpected interrupt: ");
        term_print("Frame address: 0x");
        term_print_hex((uint32_t)frame);
        term_print("\nInt No: 0x");
        term_print_hex(frame->int_no);
        term_print(" (");
        term_print_uint(frame->int_no);
        term_print(")\nError code: 0x");
        term_print_hex(frame->err_code);
        term_print("\nEIP: 0x");
        term_print_hex(frame->eip);
        term_print("\n");
    }
}
extern "C" void (*isr_stub_table[])();

extern "C" void isr_install() {
    for (int i = 0; i < 48; i++) {
        idt_set_gate(i, (uint32_t)isr_stub_table[i], 0x08, 0x8E);
    }

    // Install the system call handler
    idt_set_gate(0x80, (uint32_t)isr_stub_table[128], 0x08, 0xEE);  // User mode accessible
}