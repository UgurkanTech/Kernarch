#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include "cstring.h"


#define IDT_SIZE 256

// Structure of an IDT entry
struct IDTEntry {
    uint16_t base_low;    // Lower 16 bits of the address to jump to when this interrupt fires
    uint16_t selector;    // Kernel segment selector
    uint8_t  reserved;    // This must always be zero
    uint8_t  flags;       // More flags
    uint16_t base_high;   // The upper 16 bits of the address to jump to
} __attribute__((packed));

// Pointer to an array of IDT entries
struct IDTPointer {
    uint16_t limit;
    uint32_t base;        // The address of the first element in our idt_entry_t array
} __attribute__((packed));

extern IDTEntry idt_entries[IDT_SIZE];
extern IDTPointer idt_ptr;

// Functions implemented in idt.cpp
extern "C" void idt_flush(uint32_t idt_ptr);
extern "C" void idt_init();
extern "C" void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);

#endif // IDT_H