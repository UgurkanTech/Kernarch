#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"
#include "kernel_config.h"
#include "multiboot.h"
#include "logger.h"


void init_memory();

void multiboot_scan(multiboot_info_t* mbd, unsigned int magic);

void* kmalloc(size_t size);
void kfree(void* ptr);
void* aligned_kmalloc(size_t alignment, size_t size);
void aligned_kfree(void* ptr);
void* krealloc(void* ptr, size_t new_size);
void print_heap_info();

uint32_t get_stack_usage();

const char* get_memory_unit_text(size_t memory_size, char* buffer, size_t buffer_size);
const char* memory_info(char* buffer, size_t buffer_size);

// Declare global new and delete operators
void* operator new(size_t size) noexcept;
void* operator new[](size_t size) noexcept;
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;
void operator delete(void* ptr, size_t size) noexcept;
void operator delete[](void* ptr, size_t size) noexcept;

// Declare placement new
void* operator new(size_t, void* ptr) noexcept;

#endif // MEMORY_H