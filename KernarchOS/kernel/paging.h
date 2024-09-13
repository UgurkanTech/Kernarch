#ifndef PAGING_H
#define PAGING_H

using namespace std;

#include <stdint.h>
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
extern PageTable kernel_page_table;

void init_paging();
bool map_page(uint32_t virtual_address, uint32_t physical_address, bool is_writable);
void enable_paging();

#endif // PAGING_H