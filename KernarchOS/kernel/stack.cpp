#include "stack.h"
#include "memory.h"
#include "logger.h"

uint32_t StackManager::total_allocated = 0;

uint32_t StackManager::total_usage = 0;

Stack* StackManager::allocate_stack(uint32_t size) {
    if (size == 0) {
        Logger::log(LogLevel::ERROR, "Cannot allocate stack of size 0");
        return nullptr;
    }

    // Allocate the stack structure
    Stack* stack = new Stack();
    if (!stack) {
        Logger::log(LogLevel::ERROR, "Failed to allocate stack structure");
        return nullptr;
    }

    // Allocate memory for the stack
    void* memory = kmalloc(size);
    if (!memory) {
        Logger::log(LogLevel::ERROR, "Failed to allocate stack memory");
        delete stack;
        return nullptr;
    }

    // Initialize stack structure
    stack->base_addr = memory;
    stack->size = size;
    stack->top = (uint32_t)memory + size;
    align_stack_top(stack->top);

    // Update total allocation tracking
    total_allocated += size;

    Logger::log(LogLevel::DEBUG, "Stack allocated: base=0x%x, top=0x%x, size=%u", 
                stack->base_addr, (void*)stack->top, size);
    
    return stack;
}

void StackManager::destroy_stack(Stack* stack) {
    if (!is_valid_stack(stack)) {
        Logger::log(LogLevel::ERROR, "Attempt to destroy invalid stack");
        return;
    }
    uint32_t stack_base = (uint32_t)stack->base_addr;

    // Update total allocation tracking
    total_allocated -= stack->size;
    total_usage -= stack->usage;

    // Free the stack memory
    kfree(stack->base_addr);
    
    // Free the stack structure
    delete stack;

    Logger::log(LogLevel::DEBUG, "Stack destroyed at 0x%x", stack_base);
}

uint32_t StackManager::get_stack_usage(Stack* stack, uint32_t current_esp) {
    if (!is_address_in_stack(stack, current_esp)) {
        return 0;
    }

    total_usage -= stack->usage;
    stack->usage = stack->top - current_esp;
    total_usage += stack->usage;

    return stack->usage;
}

bool StackManager::is_stack_safe(Stack* stack, uint32_t current_esp) {
    if (!is_address_in_stack(stack, current_esp)) {
        Logger::error("ESP not in stack: ESP=0x%x, Stack=0x%x-0x%x", current_esp, stack->base_addr, stack->top);
        return false;
    }

    uint32_t usage = get_stack_usage(stack, current_esp);
    uint32_t usage_percent = (usage * 100) / stack->size;
    // Check thresholds
    if (usage_percent >= 90) {
        return false;
    }
    return true;
}

uint32_t StackManager::get_total_allocated() {
    return total_allocated;
}

uint32_t StackManager::get_total_usage() {
    return total_usage;
}

bool StackManager::is_valid_stack(Stack* stack) {
    return stack && stack->base_addr && stack->size > 0;
}

bool StackManager::is_address_in_stack(Stack* stack, uint32_t addr) {
    if (!is_valid_stack(stack)) {
        return false;
    }
    
    return addr >= (uint32_t)stack->base_addr && addr <= stack->top;
}

void StackManager::align_stack_top(uint32_t& top) {
    top &= ~(STACK_ALIGN - 1);  // Align to STACK_ALIGN bytes
}