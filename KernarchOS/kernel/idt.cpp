#include "idt.h"
#include "isr.h"

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

extern "C" void idt_init() {
    idt_ptr.limit = sizeof(IDTEntry) * 256 - 1;
    idt_ptr.base = (uint32_t)&idt_entries;

    std::memset(&idt_entries, 0, sizeof(IDTEntry) * 256);

    // Set up ISRs (0-31)
    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, (uint32_t)isr_stub_table[i], 0x08, 0x8E);
    }

    // Remap the PIC
    pic_remap(0x20, 0x28);

    // Set up software interrupts (32-255)
    for (int i = 32; i < 256; i++) {
        idt_set_gate(i, (uint32_t)isr_stub_table[i], 0x08, 0x8E);
    }

    idt_flush((uint32_t)&idt_ptr);
}