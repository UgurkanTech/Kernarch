#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// GDT entry structure
struct GDTEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

// GDT pointer structure
struct GDTPointer {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

#define GDT_ENTRIES 6

// GDT segment selectors
#define KERNEL_CODE_SEG 0x08
#define KERNEL_DATA_SEG 0x10
#define USER_CODE_SEG (0x18 | 3)  // Ring 3
#define USER_DATA_SEG (0x20 | 3)  // Ring 3
#define TSS_SEG 0x28

// Declare gdt_entries as extern
extern "C" {
    extern GDTEntry gdt_entries[GDT_ENTRIES];
    extern void gdt_flush(GDTPointer* gdt_ptr);
    extern void init_gdt();
    extern void set_tss_gdt_entry(int32_t num, uint32_t base, uint32_t limit);
}

#endif // GDT_H