#ifndef KERNEL_CONFIG_H
#define KERNEL_CONFIG_H

#define KERNEL_START 0x100000
#define KERNEL_VIRTUAL_BASE 0x00000000  // No virtual offset

#define HEAP_START_ADDRESS 0x400000  // 4MB
#define HEAP_END_ADDRESS   0xC00000  // 12MB

#define PAGE_SIZE 4096

#ifdef __cplusplus
extern "C" {
#endif

extern const uint32_t KERNEL_STACK_SIZE;
extern uint32_t stack_bottom;
extern uint32_t stack_top;

#ifdef __cplusplus
}
#endif

#endif // KERNEL_CONFIG_H