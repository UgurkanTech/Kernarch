#ifndef KERNEL_CONFIG_H
#define KERNEL_CONFIG_H

#define KERNEL_START 0x100000
#define KERNEL_VIRTUAL_BASE 0x00000000  // No virtual offset

#define PAGE_SIZE 4096


// These are defined by the linker script
extern "C" {
    extern uint32_t kernel_start;
    extern uint32_t kernel_end;
    extern uint32_t stack_guard_bottom;
    extern uint32_t stack_guard_top;
    extern uint32_t stack_bottom;
    extern uint32_t stack_top;
    extern uint32_t heap_start;
    extern uint32_t heap_end;
    extern const uint32_t KERNEL_STACK_SIZE;
}


#endif // KERNEL_CONFIG_H