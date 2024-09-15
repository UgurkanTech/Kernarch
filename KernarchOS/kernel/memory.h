#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>
#include "kernel_config.h"


void init_memory();
void* kmalloc(size_t size);
void kfree(void* ptr);
void* krealloc(void* ptr, size_t new_size);
void print_heap_info();
void print_memory_info();
uint32_t get_stack_usage();
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