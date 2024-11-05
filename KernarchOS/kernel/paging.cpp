#include "paging.h"
#include "cstring.h"
#include "terminal.h"
#include "kernel_config.h"
#include "logger.h"

PageDirectory kernel_page_directory __attribute__((aligned(4096)));
PageTable kernel_page_tables[1024] __attribute__((aligned(4096)));

void setup_stack_guard_region() {
    term_print("Setting up stack guard region...\n");

    uint32_t guard_start = (uint32_t)&stack_guard_bottom;
    uint32_t guard_end = (uint32_t)&stack_guard_top;

    for (uint32_t addr = guard_start; addr < guard_end; addr += PAGE_SIZE) {
        uint32_t pd_index = addr >> 22;
        uint32_t pt_index = (addr >> 12) & 0x3FF;

        if (pd_index < 256) {
            // Make the page present but read-only
            kernel_page_tables[pd_index].pages[pt_index] = (addr & 0xFFFFF000) | 0x5;
        } else {
            term_print("Error: Guard page outside mapped memory\n");
            break;
        }
    }

    // Refresh TLB
    asm volatile("mov %%cr3, %%eax; mov %%eax, %%cr3" : : : "eax");
    
    term_print("Stack guard region setup complete\n");
}

void init_paging() {
    term_print("Initializing paging...\n");

    // Clear the page directory
    memset(&kernel_page_directory, 0, sizeof(PageDirectory));

    // Identity map
    for (uint32_t i = 0; i < 1024; i++) {  // 32 * 4MB = 128MB
        kernel_page_directory.tables[i] = ((uint32_t)&kernel_page_tables[i]) | 3;
        kernel_page_directory.tablesPhysical[i] = ((uint32_t)&kernel_page_tables[i]) | 3;

        for (uint32_t j = 0; j < 1024; j++) {
            uint32_t addr = (i * 1024 + j) * 0x1000;
            kernel_page_tables[i].pages[j] = addr | 3; // Supervisor, read/write, present
        }
    }
    Logger::info("First 4GB memory identity mapped for kernel");

    
    // Set up user space page tables (16MB to 40MB)
    for (uint32_t i = 0; i < 1024; i++) {
        kernel_page_directory.tables[i] = ((uint32_t)&kernel_page_tables[i]) | 7; // User, read/write, present
        kernel_page_directory.tablesPhysical[i] = ((uint32_t)&kernel_page_tables[i]) | 7;

        for (uint32_t j = 0; j < 1024; j++) {
            uint32_t addr = (i * 1024 + j) * 0x1000;
            kernel_page_tables[i].pages[j] = addr | 7; // User, read/write, present
        }
    }
    Logger::info("First 4GB memory identity mapped for user space");
    
    // Set the page directory
    kernel_page_directory.physicalAddr = (uint32_t)&kernel_page_directory;

    // Load the page directory
    asm volatile("mov %0, %%cr3":: "r"(kernel_page_directory.physicalAddr));
    term_print("Page directory loaded\n");

    // Enable paging
    enable_paging();
    term_print("Paging enabled\n");

    // Set up the stack guard page after paging is enabled
    //setup_stack_guard_region();

    term_print("Paging initialization complete\n");
}

void enable_paging() {
    uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

bool map_page(uint32_t virtual_address, uint32_t physical_address, bool is_kernel, bool is_writable) {
    uint32_t pd_index = virtual_address >> 22;
    uint32_t pt_index = (virtual_address >> 12) & 0x3FF;

    if (pd_index >= 256) {
        term_print("Error: Cannot map address outside of first 1GB\n");
        return false;
    }

    PageTable* table = &kernel_page_tables[pd_index];
    uint32_t page = physical_address | 0x01; // Present
    if (is_writable) page |= 0x02; // Writable
    if (!is_kernel) page |= 0x04; // User accessible

    table->pages[pt_index] = page;

    // Invalidate TLB for this address
    asm volatile("invlpg (%0)" ::"r" (virtual_address) : "memory");

    return true;
}


uint32_t get_physical_address(uint32_t virtual_address) {
    uint32_t pd_index = virtual_address >> 22;
    uint32_t pt_index = (virtual_address >> 12) & 0x3FF;
    uint32_t offset = virtual_address & 0xFFF;

    if (pd_index >= 256) {
        term_print("Error: Address outside of mapped memory\n");
        return 0;
    }

    PageTable* table = &kernel_page_tables[pd_index];
    uint32_t page = table->pages[pt_index];

    return (page & ~0xFFF) | offset;
}

bool is_page_present(uint32_t virtual_address) {
    uint32_t pd_index = virtual_address >> 22;
    uint32_t pt_index = (virtual_address >> 12) & 0x3FF;

    if (pd_index >= 256) {
        return false;
    }

    PageTable* table = &kernel_page_tables[pd_index];
    return (table->pages[pt_index] & 0x01) != 0;
}

bool map_user_page(uint32_t virtual_address, uint32_t physical_address, bool is_writable) {
    uint32_t flags = 0x5;  // Present, User
    if (is_writable) flags |= 0x2;  // Writable
    return map_page(virtual_address, physical_address, false, flags);
}

void flush_tlb() {
    asm volatile("mov %%cr3, %%eax; mov %%eax, %%cr3" : : : "eax");
}