#include "paging.h"
#include "cstring.h"
#include "terminal.h"
#include "kernel_config.h"

PageDirectory kernel_page_directory __attribute__((aligned(4096)));
PageTable kernel_page_tables[256] __attribute__((aligned(4096)));

void init_paging() {
    term_print("Initializing paging...\n");

    // Clear the page directory
    memset(&kernel_page_directory, 0, sizeof(PageDirectory));

    // Identity map the first 1GB
    for (uint32_t i = 0; i < 256; i++) {
        kernel_page_directory.tables[i] = ((uint32_t)&kernel_page_tables[i]) | 3;
        kernel_page_directory.tablesPhysical[i] = ((uint32_t)&kernel_page_tables[i]) | 3;

        for (uint32_t j = 0; j < 1024; j++) {
            uint32_t addr = (i * 1024 + j) * 0x1000;
            kernel_page_tables[i].pages[j] = addr | 3; // Supervisor, read/write, present
        }
    }

    // Set the page directory
    kernel_page_directory.physicalAddr = (uint32_t)&kernel_page_directory;

    // Load the page directory
    asm volatile("mov %0, %%cr3":: "r"(kernel_page_directory.physicalAddr));

    // Enable paging
    enable_paging();

    term_print("Paging initialized\n");
}

void enable_paging() {
    uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

bool map_page(uint32_t virtual_address, uint32_t physical_address, bool is_writable) {
    uint32_t page_directory_index = virtual_address >> 22;
    uint32_t page_table_index = (virtual_address >> 12) & 0x3FF;

    if (page_directory_index >= 1024) {
        term_print("Error: Invalid page directory index\n");
        return false;
    }

    PageTable* table = &kernel_page_tables[page_directory_index];
    table->pages[page_table_index] = physical_address | 3 | (is_writable ? 2 : 0);
    return true;
}