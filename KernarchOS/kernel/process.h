#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "isr.h"

#define MAX_PROCESSES 256

// GDT Selectors
#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10
#define USER_CODE_SELECTOR   0x1B
#define USER_DATA_SELECTOR   0x23


typedef struct {
    // General purpose registers
    uint32_t edi;   // Offset 0
    uint32_t esi;   // Offset 4
    uint32_t ebp;   // Offset 8
    uint32_t esp;   // Offset 12
    uint32_t ebx;   // Offset 16
    uint32_t edx;   // Offset 20
    uint32_t ecx;   // Offset 24
    uint32_t eax;   // Offset 28
    
    // Segment registers
    uint16_t cs;    // Offset 32
    uint16_t ds;    // Offset 34
    uint16_t es;    // Offset 36
    uint16_t fs;    // Offset 38
    uint16_t gs;    // Offset 40
    uint16_t ss;    // Offset 42
    
    // Special registers
    uint32_t eip;   // Offset 44
    uint32_t eflags; // Offset 48
    
    // Paging registers
    uint32_t cr0;   // Offset 52
    uint32_t cr2;   // Offset 56
    uint32_t cr3;   // Offset 60
    uint32_t cr4;   // Offset 64
} Context;



enum ProcessState {
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
};

struct PCB {
    uint32_t pid;
    ProcessState state;
    uint32_t priority;
    Context context;
    uint32_t base_address;
    uint32_t limit;
    uint32_t *page_table;
    bool is_kernel_mode;
    uint32_t kernel_stack;
    uint32_t user_stack;
};

void init_processes();
PCB* create_process(void (*entry_point)(), bool is_kernel_mode);
void schedule(interrupt_frame* interrupt_frame);
void terminate_current_process();

void idle_task();
uint32_t allocate_stack();
uint32_t create_page_directory();

extern "C" void save_context(Context* context);
extern "C" void load_context(Context* context);

extern PCB process_table[MAX_PROCESSES];
extern PCB* current_process;

#endif // PROCESS_H