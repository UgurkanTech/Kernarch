#ifndef STACK_H
#define STACK_H

#include "types.h"
#include "logger.h"

struct Stack {
    void* base_addr;      // Base address of the stack
    uint32_t top;         // Stack top (ESP)
    uint32_t size;        // Total size of stack
    uint32_t usage;       // Current stack usage
};

class StackManager {
public:    
    // Stack allocation and management
    static Stack* allocate_stack(uint32_t size);
    static void destroy_stack(Stack* stack);
    
    // Stack usage tracking
    static uint32_t get_stack_usage(Stack* stack, uint32_t current_esp);
    static uint32_t get_total_allocated();
    static uint32_t get_total_usage();
    
    // Stack validation
    static bool is_valid_stack(Stack* stack);
    static bool is_address_in_stack(Stack* stack, uint32_t addr);

private:
    static const uint32_t STACK_ALIGN = 16;        // Stack alignment in bytes
    static uint32_t total_allocated;               // Total memory allocated for stacks
    static uint32_t total_usage;                   // Total stack usage
    
    static void align_stack_top(uint32_t& top);
};

#endif // STACK_H