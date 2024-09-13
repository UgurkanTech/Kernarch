#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>
#include "kernel_config.h"

// Actual variables for heap start and end
extern const uint32_t heap_start;
extern const uint32_t heap_end;

void init_memory();
void* kmalloc(size_t size);
void kfree(void* ptr);
void* krealloc(void* ptr, size_t new_size);
void print_heap_info();
void print_memory_info();

#endif // MEMORY_H