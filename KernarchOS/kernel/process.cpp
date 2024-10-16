// process.cpp
#include "process.h"
#include "memory.h"
#include "interrupts.h"
#include "paging.h"
#include "cstring.h"
#include "logger.h"
#include "tss.h"

#define STACK_SIZE 4096 // 4KB stack size

PCB process_table[MAX_PROCESSES];
PCB* current_process = nullptr;
uint32_t next_pid = 1;

void init_processes() {
    memset(process_table, 0, sizeof(process_table));
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].state = TERMINATED;
    }
    
    // Create an idle process
    PCB* idle_process = create_process(idle_task, true);
    if (idle_process) {
        idle_process->priority = 0; // Lowest priority
        Logger::log(LogLevel::INFO, "Idle process created with PID %d", idle_process->pid);
    } else {
        Logger::log(LogLevel::ERROR, "Failed to create idle process");
    }

    pit_register_scheduler(schedule);
}

void idle_task() {
    while (1) {
        __asm__ volatile("hlt");
    }
}

PCB* create_process(void (*entry_point)(), bool is_kernel_mode) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == TERMINATED) {
            PCB* pcb = &process_table[i];
            pcb->pid = next_pid++;
            pcb->state = READY;
            pcb->priority = 1; // Default priority
            pcb->is_kernel_mode = is_kernel_mode;

            // Allocate and align the kernel stack
            pcb->kernel_stack = allocate_stack();
            uint32_t kernel_stack_top = (uint32_t)pcb->kernel_stack + STACK_SIZE;
            kernel_stack_top &= ~0xF; // Align to 16 bytes

            // Allocate user stack if not kernel mode
            uint32_t user_stack_top = 0;
            if (!is_kernel_mode) {
                pcb->user_stack = allocate_stack();
                user_stack_top = (uint32_t)pcb->user_stack + STACK_SIZE;
                user_stack_top &= ~0xF; // Align to 16 bytes
            }

            // Set up the initial stack frame on the kernel stack
            uint32_t* kernel_stack_ptr = (uint32_t*)kernel_stack_top;
            if (!is_kernel_mode) {
                *(--kernel_stack_ptr) = user_stack_top;  // SS
                *(--kernel_stack_ptr) = user_stack_top;  // ESP
            }
            *(--kernel_stack_ptr) = 0x202;    // EFLAGS
            *(--kernel_stack_ptr) = is_kernel_mode ? 0x08 : 0x1B; // CS
            *(--kernel_stack_ptr) = (uint32_t)entry_point; // EIP
            *(--kernel_stack_ptr) = 0;        // Error code (pushed by CPU for some exceptions)
            *(--kernel_stack_ptr) = 0;        // EAX
            *(--kernel_stack_ptr) = 0;        // ECX
            *(--kernel_stack_ptr) = 0;        // EDX
            *(--kernel_stack_ptr) = 0;        // EBX
            *(--kernel_stack_ptr) = is_kernel_mode ? kernel_stack_top : user_stack_top; // ESP
            *(--kernel_stack_ptr) = 0;        // EBP
            *(--kernel_stack_ptr) = 0;        // ESI
            *(--kernel_stack_ptr) = 0;        // EDI

            // Set up the initial context for the new process
            memset(&pcb->context, 0, sizeof(Context));
            pcb->context.eip = (uint32_t)entry_point;
            pcb->context.esp = (uint32_t)kernel_stack_ptr;
            pcb->context.ebp = (uint32_t)kernel_stack_ptr; // Set EBP to the same as ESP initially

            // Set up segment registers
            pcb->context.cs = is_kernel_mode ? 0x08 : 0x1B; // Kernel or User code segment
            pcb->context.ds = is_kernel_mode ? 0x10 : 0x23; // Kernel or User data segment
            pcb->context.es = pcb->context.ds;
            pcb->context.fs = pcb->context.ds;
            pcb->context.gs = pcb->context.ds;
            pcb->context.ss = pcb->context.ds;

            // Set up flags (enable interrupts)
            pcb->context.eflags = 0x202; // IF flag set

            // Set up paging
            pcb->context.cr3 = kernel_page_directory.physicalAddr;

            Logger::log(LogLevel::INFO, "Created process with PID %d, EIP: 0x%x, ESP: 0x%x, Kernel: %d", 
                        pcb->pid, pcb->context.eip, pcb->context.esp, is_kernel_mode);
            return pcb;
        }
    }

    Logger::log(LogLevel::ERROR, "Failed to create process: No free PCB");
    return nullptr;
}

void schedule() {
    PCB* old_process = current_process;
    PCB* next_process = nullptr;
    
    // Simple round-robin scheduling
    for (int i = 0; i < MAX_PROCESSES; i++) {
        int index = (old_process ? (old_process - process_table + 1) % MAX_PROCESSES : 0);
        if (process_table[index].state == READY) {
            next_process = &process_table[index];
            break;
        }
    }

    // If no READY process found, use the idle process
    if (!next_process) {
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (process_table[i].priority == 0) { // Idle process
                next_process = &process_table[i];
                break;
            }
        }
    }

    // If still no process (shouldn't happen if idle process exists), log error and return
    if (!next_process) {
        Logger::log(LogLevel::ERROR, "No processes available to run, including idle process!");
        return;
    }

    // Switch to the next process
    if (next_process != old_process) {
        Logger::log(LogLevel::DEBUG, "Switching from PID %d to PID %d", 
                    old_process ? old_process->pid : 0, next_process->pid);
        
        if (old_process) {
            old_process->state = READY;
        }
        next_process->state = RUNNING;
        current_process = next_process;

        // Update TSS with new kernel stack
        tss_set_stack(next_process->kernel_stack + STACK_SIZE);
        
        Logger::log(LogLevel::DEBUG, "About to perform context switch");
        
        // Perform the actual context switch
        Context* oldContext = old_process ? &old_process->context : nullptr;
        //asm_switch_context(oldContext, &next_process->context);

        Logger::log(LogLevel::INFO, "Context switching to %d", next_process->pid);

        asm volatile("cli");
        save_context(oldContext);  // Save the current process state
        load_context(&next_process->context);     // Load the new process state
        asm volatile("sti");

        Logger::log(LogLevel::INFO, "Context switched to %d", next_process->pid);
        
        Logger::log(LogLevel::DEBUG, "Returned from context switch");
    } else {
        Logger::log(LogLevel::DEBUG, "No context switch needed");
    }
}

uint32_t allocate_stack() {
    void* stack = kmalloc(STACK_SIZE);
    if (!stack) {
        Logger::log(LogLevel::ERROR, "Failed to allocate stack");
        return 0;
    }
    return (uint32_t)stack;
}

uint32_t create_page_directory() {
    uint32_t* page_dir = (uint32_t*)kmalloc(4096); // 4KB aligned
    if (!page_dir) {
        Logger::log(LogLevel::ERROR, "Failed to allocate page directory");
        return 0;
    }
    
    // Initialize page directory
    memset(page_dir, 0, 4096);
    
    // Map all 4GB of address space
    for (uint32_t i = 0; i < 1024; i++) {
        // 0x87: Present, Read/Write, User, 4MB pages
        page_dir[i] = (i * 0x400000) | 0x87;
    }
    
    Logger::log(LogLevel::INFO, "Created page directory at 0x%x", (uint32_t)page_dir);
    
    return (uint32_t)page_dir;
}

void terminate_process(PCB* process) {
    if (!process) return;
    
    // Free the stack
    kfree((void*)(process->context.esp - STACK_SIZE));
    
    // Free the page directory
    //kfree((void*)process->context.cr3);
    
    // Mark the process as terminated
    process->state = TERMINATED;
    process->pid = 0;
    
    Logger::log(LogLevel::INFO, "Terminated process with PID %d", process->pid);
    
    if (process == current_process) {
        current_process = nullptr;
        schedule(); // Schedule a new process
    }
}