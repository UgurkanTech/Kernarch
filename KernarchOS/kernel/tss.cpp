#include "tss.h"
#include "cstring.h"

TSS tss;

#define IO_PERMISSION_MAP_SIZE 8192 // Size in bytes for 8192 ports


void init_tss() {
    // Zero out the TSS
    memset(&tss, 0, sizeof(tss));
    
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(tss);

    // Initialize TSS
    tss.ss0 = KERNEL_DATA_SEG; // Stack segment for privilege level 0

    tss.cs = USER_CODE_SEG | 3; // Code segment for user mode 
    tss.ds = tss.es = tss.fs = tss.gs = USER_DATA_SEG | 3; // Data segments for user mode 
    tss.ss = USER_DATA_SEG | 3;
    
    memset(tss.io_bitmap, 0x00, sizeof(tss.io_bitmap));

    tss.iomap_base = (uint32_t)(&tss.io_bitmap) - (uint32_t)(&tss); // Set the base address of the I/O permission map
     
     // Set up the TSS entry in the GDT
    gdt_set_gate(5, base, limit, 0x89, 0x00);
    // Load the TSS
    asm volatile ("ltr %%ax" : : "a" (TSS_SEG));
}

void tss_set_stack(uint32_t kesp) {
    tss.esp0 = kesp;
}