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
void* krealloc(void* ptr, size_t new_size);
void print_heap_info();
void print_memory_info();
uint32_t get_stack_usage();

void print_memory_size(const char* prefix, size_t size);

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