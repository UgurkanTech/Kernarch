#include "memory.h"
#include "terminal.h"
#include "interrupts.h"
#include "cstring.h"
#include <stdint.h>

using namespace std;

// Define the actual variables
const uint32_t heap_start = HEAP_START_ADDRESS;
const uint32_t heap_end = HEAP_END_ADDRESS;

#define ALIGN_UP(num, align) (((num) + ((align) - 1)) & ~((align) - 1))
#define MIN_BLOCK_SIZE 16

struct block_meta {
    size_t size;
    bool free;
    block_meta* next;
    block_meta* prev;
};

static block_meta* heap_start_block = nullptr;

void init_memory() {
    term_print("Initializing memory...\n");

    heap_start_block = (block_meta*)heap_start;
    size_t heap_size = heap_end - heap_start;

    heap_start_block->size = heap_size - sizeof(block_meta);
    heap_start_block->free = true;
    heap_start_block->next = nullptr;
    heap_start_block->prev = nullptr;

    term_print("Heap initialized. Start: ");
    term_print_hex((uint32_t)heap_start_block);
    term_print(", Size: ");
    term_print_int(heap_start_block->size);
    term_print(" bytes\n");
}

void* kmalloc(size_t size) {
    if (size == 0) return nullptr;

    size = ALIGN_UP(size, sizeof(void*));  // Align to pointer size
    
    block_meta* best_fit = nullptr;
    block_meta* current = heap_start_block;

    while (current) {
        if (current->free && current->size >= size) {
            if (!best_fit || current->size < best_fit->size) {
                best_fit = current;
                // If we find an exact fit, use it immediately
                if (current->size == size) {
                    break;
                }
            }
        }
        current = current->next;
    }

    if (best_fit) {
        if (best_fit->size > size + sizeof(block_meta) + MIN_BLOCK_SIZE) {
            // Split the block only if the remaining piece is large enough
            block_meta* new_block = (block_meta*)((char*)best_fit + sizeof(block_meta) + size);
            new_block->size = best_fit->size - size - sizeof(block_meta);
            new_block->free = true;
            new_block->next = best_fit->next;
            new_block->prev = best_fit;
            if (best_fit->next) best_fit->next->prev = new_block;
            best_fit->next = new_block;
            best_fit->size = size;
        }
        best_fit->free = false;
        return (char*)best_fit + sizeof(block_meta);
    }

    // If we reach here, we couldn't find a suitable block
    term_print("kmalloc failed: Out of memory. Requested size: ");
    term_print_int(size);
    term_print("\n");

    // Trigger software interrupt for out of memory
    trigger_interrupt(INT_OUT_OF_MEMORY);

    return nullptr;
}

void kfree(void* ptr) {
    if (!ptr) return;

    block_meta* block = (block_meta*)((char*)ptr - sizeof(block_meta));
    block->free = true;

    // Only coalesce with next block if it's free
    if (block->next && block->next->free) {
        block->size += block->next->size + sizeof(block_meta);
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }

    // Only coalesce with previous block if it's free
    if (block->prev && block->prev->free) {
        block->prev->size += block->size + sizeof(block_meta);
        block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;
    }
}

void* krealloc(void* ptr, size_t new_size) {
    if (!ptr) return kmalloc(new_size);
    if (new_size == 0) {
        kfree(ptr);
        return nullptr;
    }

    block_meta* block = (block_meta*)((char*)ptr - sizeof(block_meta));
    if (block->size >= new_size) return ptr; // No need to reallocate

    void* new_ptr = kmalloc(new_size);
    if (!new_ptr) return nullptr; // Out of memory

    memcpy(new_ptr, ptr, block->size);
    kfree(ptr);
    return new_ptr;
}

void print_heap_info() {
    term_print("Heap info:\n");
    block_meta* current = heap_start_block;
    int block_count = 0;
    size_t free_memory = 0;
    size_t used_memory = 0;

    while (current) {
        block_count++;
        term_print("  Block ");
        term_print_int(block_count);
        term_print(": Address ");
        term_print_hex((uint32_t)current);
        term_print(", Size ");
        term_print_int(current->size);
        term_print(", Is Free ");
        term_print_int(current->free);
        term_print("\n");

        if (current->free) {
            free_memory += current->size;
        } else {
            used_memory += current->size;
        }
        current = current->next;
    }

    term_print("  Total blocks: ");
    term_print_int(block_count);
    term_print("\n  Free memory: ");
    term_print_int(free_memory);
    term_print(" bytes\n  Used memory: ");
    term_print_int(used_memory);
    term_print(" bytes\n");
}

#include "kernel_config.h"

uint32_t get_stack_usage() {
    uint32_t esp;
    asm volatile("mov %%esp, %0" : "=r" (esp));
    return (uint32_t)(&stack_top - (uint32_t*)esp);
}

uint32_t get_total_stack_size() {
    return (uint32_t)(&stack_top - &stack_bottom);
}

void print_memory_info(){
    term_print("Heap info:\n");
    block_meta* current = heap_start_block;
    int block_count = 0;
    size_t free_memory = 0;
    size_t used_memory = 0;

    while (current) {
        block_count++;
        if (current->free) {
            free_memory += current->size;
        } else {
            used_memory += current->size;
        }
        current = current->next;
    }
    term_print("  Blocks: ");
    term_print_int(block_count);

    term_print("\n  Used: ");
    
    if (used_memory > 1024 * 1024) {
        term_print_int(used_memory / 1024 / 1024);
        term_print(" MB\n");
    } else if (used_memory > 1024) {
        term_print_int(used_memory / 1024);
        term_print(" KB\n");
    } else {
        term_print_int(used_memory);
        term_print(" bytes\n");
    }


    term_print("  Free: ");
    if (free_memory > 1024 * 1024) {
        term_print_int(free_memory / 1024 / 1024);
        term_print(" MB\n");
    } else if (free_memory > 1024) {
        term_print_int(free_memory / 1024);
        term_print(" KB\n");
    } else {
        term_print_int(free_memory);
        term_print(" bytes\n");
    }


    term_print("  Total: ");
    size_t total_memory = free_memory + used_memory;
    if (total_memory > 1024 * 1024) {
        term_print_int(total_memory / 1024 / 1024);
        term_print(" MB\n");
    } else if (total_memory > 1024) {
        term_print_int(total_memory / 1024);
        term_print(" KB\n");
    } else {
        term_print_int(total_memory);
        term_print(" bytes\n");
    }

    term_print("Stack info: \n  Used: ");
    term_print_int(get_stack_usage());
    term_print(" bytes\n  Total: ");
    term_print_int(get_total_stack_size());
    term_print(" bytes\n");


}

