#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

#define MAX_PROCESSES 256

// GDT Selectors
#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10
#define USER_CODE_SELECTOR   0x1B
#define USER_DATA_SELECTOR   0x23


typedef struct {
    // General purpose registers
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    
    // Segment registers
    uint16_t cs, ds, es, fs, gs, ss;
    
    // Special registers
    uint32_t eip;
    uint32_t eflags;
    
    // Paging registers
    uint32_t cr0, cr2, cr3, cr4;
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
void schedule();
void terminate_process(PCB* process);

void idle_task();
uint32_t allocate_stack();
uint32_t create_page_directory();

extern "C" void save_context(Context* context);
extern "C" void load_context(Context* context);

extern PCB process_table[MAX_PROCESSES];
extern PCB* current_process;

#endif // PROCESS_H