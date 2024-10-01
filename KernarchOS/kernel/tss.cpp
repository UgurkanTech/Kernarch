#include "tss.h"
#include <cstring>

TSS tss;

void init_tss(uint32_t kernel_stack) {
    // Zero out the TSS
    memset(&tss, 0, sizeof(tss));
    
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(tss);

    // Initialize TSS
    tss.ss0 = KERNEL_DATA_SEG; // Stack segment for privilege level 0
    tss.esp0 = kernel_stack;    // Stack pointer for privilege level 0
    tss.cs = USER_CODE_SEG | 3; // Code segment for user mode
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = USER_DATA_SEG | 3; // Data segments for user mode
    tss.iomap_base = limit; // I/O map base address

     // Set up the TSS entry in the GDT
    set_tss_gdt_entry(5, base, limit);

    // Load the TSS
    asm volatile ("ltr %%ax" : : "a" (TSS_SEG));
}

void tss_set_stack(uint32_t kss, uint32_t kesp) {
    tss.ss0 = kss;
    tss.esp0 = kesp;
}