#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "isr.h"
#include "stack.h"

#define MAX_PROCESSES 256

// GDT Selectors
#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10
#define USER_CODE_SELECTOR   0x1B
#define USER_DATA_SELECTOR   0x23


enum ProcessState {
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
};

struct Thread;

typedef struct PCB {
    uint32_t pid;
    ProcessState state;
    uint32_t priority;
    interrupt_frame context;
    uint32_t base_address;
    uint32_t limit;
    uint32_t *page_table;
    Stack* kernel_stack;
    Stack* user_stack;
    uint8_t* fpu_state;
    Thread* user_data;
};

void init_processes();
PCB* create_process(void (*entry_point)());
void schedule(interrupt_frame* interrupt_frame);
void terminate_current_process(int return_code = 0);

void idle_task();

extern "C" void load_context(interrupt_frame* context);

extern PCB process_table[MAX_PROCESSES];
extern PCB* current_process;

#endif // PROCESS_H