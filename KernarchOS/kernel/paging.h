#ifndef PAGING_H
#define PAGING_H

#include "types.h"
#include "kernel_config.h"

#define PAGE_SIZE 4096
#define TABLE_SIZE 1024

struct PageTable {
    uint32_t pages[1024];
};

struct PageDirectory {
    uint32_t tables[1024];
    uint32_t tablesPhysical[1024];
    uint32_t physicalAddr;
};

// Statically allocate the kernel page directory and initial page table
extern PageDirectory kernel_page_directory;
extern PageTable kernel_page_tables[1024];

void init_paging();
bool map_page(uint32_t virtual_address, uint32_t physical_address, bool is_kernel, bool is_writable);
void setup_stack_guard_region();
void enable_paging();
uint32_t get_physical_address(uint32_t virtual_address);
bool is_page_present(uint32_t virtual_address);
bool map_user_page(uint32_t virtual_address, uint32_t physical_address, bool is_writable);
void flush_tlb();

#endif // PAGING_H