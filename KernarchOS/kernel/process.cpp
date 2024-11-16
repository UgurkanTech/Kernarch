// process.cpp
#include "process.h"
#include "memory.h"
#include "paging.h"
#include "cstring.h"
#include "logger.h"
#include "tss.h"
#include "io.h"
#include "pit.h"

#define STACK_SIZE 8192 // 8KB stack size

PCB process_table[MAX_PROCESSES];
PCB* current_process = nullptr;
uint32_t next_pid = 0;


void init_processes() {
    memset(process_table, 0, sizeof(process_table));
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].state = TERMINATED;
    }
    
    // Create an idle process
    PCB* idle_process = create_process(idle_task, true);
    if (idle_process) {
        idle_process->priority = 0;  // Lowest priority
        idle_process->state = READY; // Ensure it's ready
        
        // Make sure interrupts are enabled in the idle process context
        idle_process->context.eflags |= 0x200;  // Set IF flag
        
        Logger::log(LogLevel::INFO, "Idle process created with PID %d", idle_process->pid);
    } else {
        Logger::log(LogLevel::ERROR, "Failed to create idle process");
    }

    pit_register_scheduler(schedule);
}

#define SYSCALL_YIELD 0x80

void idle_task() {
    while (1) {
        asm("pause");
    }
}


PCB* create_process(void (*entry_point)(), bool is_kernel_mode) {
    // Find free PCB
    PCB* pcb = nullptr;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == TERMINATED) {
            pcb = &process_table[i];
            break;
        }
    }
    
    if (!pcb) {
        Logger::log(LogLevel::ERROR, "Failed to create process: No free PCB");
        return nullptr;
    }

    // Initialize PCB
    pcb->pid = next_pid++;
    pcb->state = READY;
    pcb->priority = 1;
    pcb->is_kernel_mode = is_kernel_mode;

    // Allocate and align kernel stack
    pcb->kernel_stack = allocate_stack();
    uint32_t kernel_stack_top = (uint32_t)pcb->kernel_stack + STACK_SIZE;
    kernel_stack_top &= ~0xF;  // 16-byte align

    // For user mode, allocate user stack
    uint32_t user_stack_top = 0;
    if (!is_kernel_mode) {
        pcb->user_stack = allocate_stack();
        user_stack_top = (uint32_t)pcb->user_stack + STACK_SIZE;
        user_stack_top &= ~0xF;
    }

    // Initialize context
    memset(&pcb->context, 0, sizeof(interrupt_frame));
    
    // Set up registers
    pcb->context.eip = (uint32_t)entry_point;
    pcb->context.esp = is_kernel_mode ? kernel_stack_top : user_stack_top;
    pcb->context.ebp = pcb->context.esp;  // Initial stack frame
    
    // Set up segments
    pcb->context.cs = is_kernel_mode ? 0x08 : 0x1B;
    pcb->context.ds = is_kernel_mode ? 0x10 : 0x23;
    pcb->context.es = pcb->context.ds;
    pcb->context.fs = pcb->context.ds;
    pcb->context.gs = pcb->context.ds;
    pcb->context.ss = pcb->context.ds;
    
    // Enable interrupts in flags
    pcb->context.eflags = 0x200;
    
    // Set page directory
    //pcb->context.cr3 = kernel_page_directory.physicalAddr;

    Logger::log(LogLevel::INFO, "Created process PID %d, EIP: 0x%x, ESP: 0x%x, Kernel: %d",
                pcb->pid, pcb->context.eip, pcb->context.esp, is_kernel_mode);


    //serial_log("Created process PID %d \n", pcb->pid);
    //print_context(&pcb->context);
    
    return pcb;
}



void schedule(interrupt_frame* interrupt_frame) {
    //Logger::log(LogLevel::ERROR, "schedule init");
    
    // Save CPU state and disable interrupts atomically
    uint32_t eflags = 0x200;
    asm volatile("cli");

    PCB* old_process = current_process;
    PCB* next_process = nullptr;
    
    // Try to find next READY process
    int start_idx = old_process ? (old_process->pid % MAX_PROCESSES) : 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        int idx = (start_idx + i) % MAX_PROCESSES;
        if (process_table[idx].state == READY && process_table[idx].priority > 0){
            next_process = &process_table[idx];
            break;
        }
    }

    // If not READY, maybe one running? Keep it running.
    if (!next_process) {
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (process_table[i].priority == 1 && process_table[i].state == RUNNING) {
                if(old_process->pid == process_table[i].pid){
                    asm volatile("sti");
                    return;
                }
                    
                break;
            }
        }
    }

        // If no READY process, use idle
    if (!next_process) {
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (process_table[i].priority == 0 && process_table[i].state == READY) {
                next_process = &process_table[i];
                break;
            }
        }
    }

    if (!next_process) {
        if (old_process->pid == 0)
            Logger::log(LogLevel::DEBUG, "System is idle.");
        else   
            Logger::log(LogLevel::ERROR, "No processes available to execute!");

        asm volatile("sti");
        return;
    }

    // Don't switch if it's the same process
    if (next_process == old_process) return;

    // Update process states
    if (old_process && old_process->state == RUNNING) {
        old_process->state = READY;
    }
    next_process->state = RUNNING;  // Mark the next process as RUNNING
    current_process = next_process;

    //term_print("\n");
    //Logger::log(LogLevel::DEBUG, "Scheduling Thread %d (%s)", next_process->pid, next_process->is_kernel_mode ? "Kernel" : "User");

    uint32_t esp;

    Logger::serial_log("================================\n");
    
    asm volatile ("mov %%esp, %0" : "=r"(esp));
    Logger::serial_log("BEFORE KERNEL ESP: 0x%x \n", esp);

    if (old_process) {
        interruptFrame* old_ctx = &old_process->context;    
        *old_ctx = *interrupt_frame;

        old_ctx->ss = old_process->is_kernel_mode ? 0x10 : 0x23;
        old_ctx->cs = old_process->is_kernel_mode ? 0x08 : 0x1B;

        // Calculate stack usage
        uint32_t stack_usage = (((uint32_t)old_process->user_stack + STACK_SIZE) & ~0xF) - (uint32_t)old_ctx->esp;
        Logger::serial_log("Stack Usage: %d bytes\n", stack_usage);


        Logger::serial_log("Saved context for process PID %d \n", old_process->pid);
        //print_interrupt_frame(&old_process->context); // Pass the context directly
        Logger::serial_log("Saved ESP: 0x%x \n", old_process->context.esp);
    }

    //asm volatile ("mov %%esp, %0" : "=r"(esp));
    //serial_log("AFTER KERNEL ESP: 0x%x \n", esp);

    // Load the new process's context
    if (next_process) {
        Logger::serial_log("Loading process PID %d \n", next_process->pid);
        //print_interrupt_frame(&next_process->context);

        Logger::serial_log("Loading ESP: 0x%x \n", next_process->context.esp);
        uint32_t kernel_stack_top = (uint32_t)next_process->kernel_stack + STACK_SIZE;
        kernel_stack_top &= ~0xF;  // 16-byte align
        tss_set_stack(kernel_stack_top);
        load_context(&next_process->context); // Load the context of the next process
    }

    Logger::serial_log("scheduler end reached !?!?");
}

uint32_t allocate_stack() {
    void* stack = kmalloc(STACK_SIZE);
    if (!stack) {
        Logger::log(LogLevel::ERROR, "Failed to allocate stack");
        return 0;
    }
    return (uint32_t)stack;
}


void terminate_current_process() {
    if (current_process && current_process->pid != 0) {  // Don't terminate idle process
        current_process->state = TERMINATED;
        
        // Free process resources
        if (current_process->kernel_stack) {
            kfree((void*)current_process->kernel_stack);
            current_process->kernel_stack = NULL;
        }
        if (current_process->user_stack) {
            kfree((void*)current_process->user_stack);
            current_process->user_stack = NULL;
        }
        Logger::log(LogLevel::INFO, "Terminated process PID %d", current_process->pid);
        
        current_process = nullptr;

        // Trigger a reschedule to pick the next process
        schedule(nullptr);
    }
}