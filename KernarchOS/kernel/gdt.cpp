#include "gdt.h"

extern "C" GDTEntry gdt_entries[GDT_ENTRIES];

static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt_entries[num].access      = access;
}

void init_gdt() {
    GDTPointer gdt_ptr;
    gdt_ptr.limit = (sizeof(GDTEntry) * GDT_ENTRIES) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0); // Null segment
    
    // Base: 0, Limit: 0xFFFFFFFF (4096MB)
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel Code segment - 0x9A (present, executable, readable, DPL = 0)
    // Base: 0, Limit: 0xFFFFFFFF (4096MB)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Kernel Data segment - 0x92 (present, writable, DPL = 0)
    
    // Base: 0x1000000 (16MB), Limit: 0xFFFFFFFF (4096MB)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment - 0xFA (present, executable, readable, DPL = 3)
    // Base: 0x1000000 (16MB), Limit: 0xFFFFFFFF (4096MB)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment - 0xF2 (present, writable, DPL = 3)

    gdt_flush(&gdt_ptr);
}

void set_tss_gdt_entry(int32_t num, uint32_t base, uint32_t limit) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;
    
    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = ((limit >> 16) & 0x0F) | 0x40; // Set granularity to 4KB
    gdt_entries[num].access      = 0xE9; // Access byte for TSS
}