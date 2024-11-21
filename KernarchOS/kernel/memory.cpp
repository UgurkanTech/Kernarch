#include "memory.h"
#include "terminal.h"
#include "interrupts.h"
#include "cstring.h"
#include "kernel_config.h"

using namespace std;


#define ALIGN_UP(num, align) (((num) + ((align) - 1)) & ~((align) - 1))
#define MIN_BLOCK_SIZE 16

uintptr_t kmalloc_start = 0;
size_t kmalloc_size = 0;

struct block_meta {
    size_t size;
    bool free;
    block_meta* next;
    block_meta* prev;
};

static block_meta* heap_start_block = nullptr;

void init_memory() {
    Logger::log(LogLevel::INFO, "Initializing memory...");

    if (kmalloc_size == 0)
    {
        Logger::log(LogLevel::ERROR, "Failed to initialize memory. Is multiboot scanned?");
        return;
    }
    
    // Align the start address to a page boundary
    uintptr_t start_addr = ALIGN_UP(kmalloc_start, 4096);

    heap_start_block = (block_meta*)start_addr;
    size_t heap_size = kmalloc_size;

    heap_start_block->size = heap_size - sizeof(block_meta);
    heap_start_block->free = true;
    heap_start_block->next = nullptr;
    heap_start_block->prev = nullptr;

    Logger::log(LogLevel::INFO, "Heap initialized. Start: 0x%x, Size: %d bytes", 
                (uint32_t)heap_start_block, heap_start_block->size);
}

void multiboot_scan(multiboot_info_t* mbd, unsigned int magic){

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        Logger::error("Invalid magic number!");
        return;
    }

    if (!(mbd->flags >> 6 & 0x1)) {
        Logger::error("Invalid memory map from GRUB Bootloader!");
        return;
    }

    uintptr_t kernel_end = USER_SPACE_START; // Assuming you have this symbol defined in your linker script
    kernel_end = ALIGN_UP(kernel_end, 4096); // Align to page boundary

    Logger::log(LogLevel::INFO, "Scanning memory map...");

    multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mbd->mmap_addr;
    while ((uintptr_t)mmap < mbd->mmap_addr + mbd->mmap_length) {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            uintptr_t region_start = (uintptr_t)mmap->addr;
            uintptr_t region_end = region_start + mmap->len;

            // Ensure we're not using memory below the kernel
            if (region_start < kernel_end) {
                region_start = kernel_end;
            }

            // Recalculate region size after potential adjustment
            size_t region_size = region_end > region_start ? region_end - region_start : 0;

            // Check if this region is larger than the previously found largest region
            if (region_size > kmalloc_size) {
                kmalloc_start = region_start;
                kmalloc_size = region_size;
            }
        }

        mmap = (multiboot_memory_map_t*)((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
    }

    if (kmalloc_size == 0) {
        Logger::error("No suitable memory region found for kmalloc!");
        return;
    }

    Logger::log(LogLevel::INFO, "Largest available memory region for kmalloc: 0x%x, Size: %d bytes", kmalloc_start, kmalloc_size);

    return; //Return for now..

    //Loop  the memory map and display the values
    for(int i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) 
    {
        multiboot_memory_map_t* mmmt = (multiboot_memory_map_t*) (mbd->mmap_addr + i);

        term_printf("Start Addr: %x | Length: %x | Size: %x | Type: %x\n", mmmt->addr, mmmt->len, mmmt->size, mmmt->type);

        if(mmmt->type == MULTIBOOT_MEMORY_AVAILABLE) {

        }
    }
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
    term_printf("%d", size);
    term_print("\n");

    // Trigger software interrupt for out of memory
    
    //trigger_interrupt(INT_OUT_OF_MEMORY); //FIX THIS!

    return nullptr;
}

void* aligned_kmalloc(size_t alignment, size_t size) {
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        // Alignment must be a power of 2
        return nullptr;
    }

    // Allocate extra space for alignment and storing the original pointer
    size_t total_size = size + alignment + sizeof(void*);
    void* original_ptr = kmalloc(total_size);
    if (!original_ptr) {
        return nullptr;
    }

    // Calculate aligned address
    uintptr_t original_addr = (uintptr_t)original_ptr;
    uintptr_t aligned_addr = (original_addr + sizeof(void*) + alignment - 1) & ~(alignment - 1);
    
    // Store the original pointer right before the aligned memory
    void** storage = (void**)(aligned_addr - sizeof(void*));
    *storage = original_ptr;

    return (void*)aligned_addr;
}

void aligned_kfree(void* ptr) {
    if (!ptr) return;

    // Get the original pointer from before the aligned memory
    void** storage = (void**)((uintptr_t)ptr - sizeof(void*));
    void* original_ptr = *storage;

    // Free the original allocation
    kfree(original_ptr);
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
        term_printf("  Block %d : Address %x, Size %d, Is Free %d \n", block_count, (uint32_t)current, current->size, current->free);

        if (current->free) {
            free_memory += current->size;
        } else {
            used_memory += current->size;
        }
        current = current->next;
    }

    term_printf("  Total blocks: %d \n", block_count);
    term_printf("  Free memory: %d \n", free_memory);
    term_printf("  Used memory: %d \n", used_memory);
}


const char* get_memory_unit_text(size_t memory_size, char* buffer, size_t buffer_size) {
    if (memory_size >= 1024 * 1024) {
        format_string(buffer, buffer_size, "%d MB", memory_size / (1024 * 1024));
    } else if (memory_size >= 1024) {
        format_string(buffer, buffer_size, "%d KB", memory_size / 1024);
    } else {
        format_string(buffer, buffer_size, "%d bytes", memory_size);
    }
    return buffer;
}

const char* memory_info(char* buffer, size_t buffer_size) {
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
    size_t total_heap_size = free_memory + used_memory + (block_count * sizeof(block_meta));

    char unit_buffer1[32];
    char unit_buffer2[32];
    char unit_buffer3[32];

    format_string(buffer, buffer_size, "&9Total Heap Size: &f%s\n &cUsed Memory: &f%s\n &aFree Memory: &f%s\n &eBlock Count: &f%d", 
                    get_memory_unit_text(total_heap_size, unit_buffer1, sizeof(unit_buffer1)),
                    get_memory_unit_text(used_memory, unit_buffer2, sizeof(unit_buffer2)),
                    get_memory_unit_text(free_memory, unit_buffer3, sizeof(unit_buffer3)),
                    block_count);
    return buffer;
}

// Global new and delete operators
void* operator new(size_t size) noexcept {
    return kmalloc(size);
}

void* operator new[](size_t size) noexcept {
    return kmalloc(size);
}

void operator delete(void* ptr) noexcept {
    kfree(ptr);
}

void operator delete[](void* ptr) noexcept {
    kfree(ptr);
}

void operator delete(void* ptr, size_t size) noexcept {
    (void)size;
    kfree(ptr);
}

void operator delete[](void* ptr, size_t size) noexcept {
    (void)size;
    kfree(ptr);
}

// Placement new
void* operator new(size_t, void* ptr) noexcept {
    return ptr;
}