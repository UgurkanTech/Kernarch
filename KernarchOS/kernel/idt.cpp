#include "idt.h"

// Declare an IDT of 256 entries. Although we will only use the
// first 32 entries in this tutorial, the rest exists as a bit
// of a trap. If any undefined IDT entry is hit, it normally
// will cause an "Unhandled Interrupt" exception. Any descriptor
// for which the 'presence' bit is cleared (0) will generate an
// "Unhandled Interrupt" exception.

// Define idt_entries and idt_ptr
IDTEntry idt_entries[IDT_SIZE];
IDTPointer idt_ptr;

extern "C" void idt_flush(uint32_t idt_ptr) {
    asm volatile("lidtl (%0)" : : "r" (idt_ptr));
}

// Set the value of one IDT entry.
extern "C" void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt_entries[num].base_low = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].selector = selector;
    idt_entries[num].reserved = 0;
    idt_entries[num].flags = flags;
}

// Define isr0 and isr1
extern "C" void isr0() {
    // ISR handler code goes here
}

extern "C" void isr1() {
    // ISR handler code goes here
}

// Initialization of IDT
extern "C" void idt_init() {
    idt_ptr.limit = sizeof(IDTEntry) * IDT_SIZE - 1;
    idt_ptr.base = (uint32_t)&idt_entries;

    // Clear out the entire IDT, initializing it to zeros
    memset(&idt_entries, 0, sizeof(IDTEntry) * IDT_SIZE);

    // Add any new ISRs to the IDT here using idt_set_gate
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);

    // Points the processor's internal register to the new IDT
    idt_flush((uint32_t)&idt_ptr);
}