#include "tss.h"
#include <cstring>

TSS tss;

extern "C" void set_tss_gdt_entry(int32_t num, uint32_t base, uint32_t limit);


void init_tss(uint32_t kernel_stack) {
    // Zero out the TSS
    memset(&tss, 0, sizeof(tss));
    
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(tss);

    // Initialize TSS
    tss.ss0 = KERNEL_DATA_SEG;
    tss.esp0 = kernel_stack;
    tss.cs = USER_CODE_SEG | 3;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = USER_DATA_SEG | 3;
    tss.iomap_base = sizeof(tss);

    // Set up the TSS entry in the GDT
    gdt_entries[5].base_low    = base & 0xFFFF;
    gdt_entries[5].base_middle = (base >> 16) & 0xFF;
    gdt_entries[5].base_high   = (base >> 24) & 0xFF;
    gdt_entries[5].limit_low   = limit & 0xFFFF;
    gdt_entries[5].granularity = ((limit >> 16) & 0x0F) | 0x40;
    gdt_entries[5].access      = 0x89;

     // Set up the TSS entry in the GDT
    set_tss_gdt_entry(5, (uint32_t)&tss, sizeof(tss));

    // Load the TSS
    asm volatile ("ltr %%ax" : : "a" (TSS_SEG));
}