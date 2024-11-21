// process.cpp
#include "process.h"
#include "memory.h"
#include "paging.h"
#include "cstring.h"
#include "logger.h"
#include "tss.h"
#include "io.h"
#include "pit.h"
#include "thread.h"
#include "interrupts.h"

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

    Thread* idleThread = ThreadManager::create_thread(idle_task);

    if (idleThread)
    {
        idleThread->pcb->priority = 0;
        idleThread->pcb->state = READY;

        idleThread->pcb->context.eflags |= 0x200;

        Logger::log(LogLevel::INFO, "Idle process created with PID %d", idleThread->pcb->pid);
    }
    else
    {
        Logger::log(LogLevel::ERROR, "Failed to create idle process");
    }
    
    pit_register_scheduler(schedule);
}

#define SYSCALL_YIELD 0x80

void idle_task() {
    while (1) {
        asm volatile("int $0xFF");
    }
}

PCB* create_process(void (*entry_point)()) {
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
    pcb->fpu_state = nullptr;

        // Initialize context
    memset(&pcb->context, 0, sizeof(interrupt_frame));

    // Always allocate kernel stack for both Ring 0 and Ring 3
    //pcb->kernel_stack = StackManager::allocate_stack(STACK_SIZE); // Not used for now

    // Always allocate user/task stack for both Ring 0 and Ring 3
    pcb->user_stack  = StackManager::allocate_stack(STACK_SIZE);

    pcb->context.esp = pcb->user_stack->top;
    pcb->context.ebp = pcb->user_stack->top;  // Initial stack frame
    
    // Set up registers - always use task stack as main stack
    pcb->context.eip = (uint32_t)entry_point;

    
    // Set up segments
    pcb->context.cs = 0x1B;  // CS: Kernel or User code segment
    pcb->context.ds = pcb->context.es = pcb->context.fs = pcb->context.gs = 0x23;  // DS, ES, FS, GS
    pcb->context.ss = 0x23;  // SS: Stack segment for Ring 0 or Ring 3
    
    // Enable interrupts
    pcb->context.eflags = 0x202;  // IF + bit 1 (reserved)

    // Set page directory
    //pcb->context.cr3 = kernel_page_directory.physicalAddr;

    Logger::log(LogLevel::INFO, "Created process PID %d, EIP: 0x%x, ESP: 0x%x",
                pcb->pid, pcb->context.eip, pcb->context.esp);

    //serial_log("Created process PID %d \n", pcb->pid);
    //print_context(&pcb->context);
    
    return pcb;
}


void schedule(interrupt_frame* interrupt_frame) {
    uint32_t eflags;
    asm volatile("pushf; pop %0" : "=r"(eflags));
    asm volatile("cli");

    if (current_process && current_process->user_data->state == THREAD_TERMINATED)
    {
        ThreadManager::exit_thread();
    }

    // Update sleeping threads
    ThreadManager::update_sleeping_threads();

    PCB* old_process = current_process;
    PCB* next_process = nullptr;

    if(old_process && !StackManager::is_stack_safe(old_process->user_stack, interrupt_frame->esp)){
        Logger::log(LogLevel::ERROR, "Stack overflow detected for process PID %d", old_process->pid);
        Logger::log(LogLevel::ERROR, "Stack top: 0x%x, ESP: 0x%x", old_process->user_stack->top, interrupt_frame->esp);
        //ThreadManager::exit_thread();
    }
    
    // Try to find the next READY process
    for (int i = 0; i < MAX_PROCESSES; i++) {
        int idx = (i + (old_process ? (old_process->pid % MAX_PROCESSES) : 0)) % MAX_PROCESSES;
        if (process_table[idx].state == READY) {
            Thread* thread = (Thread*)process_table[idx].user_data;
            if (thread && ThreadManager::is_thread_ready(thread)) {
                next_process = &process_table[idx];
                break;
            }
        }
    }

    // If no READY process, check for the idle task
    if (!next_process) {
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (process_table[i].priority == 0 && process_table[i].state == READY) {
                next_process = &process_table[i];
                break;
            }
        }
    }

    if (!next_process) {
        if (old_process->pid != 0)
            Logger::log(LogLevel::ERROR, "No processes available to execute!");

        asm volatile("sti");
        return;
    }

    // Don't switch if it's the same process
    if (next_process == old_process) {
        asm volatile("sti");
        return;
    }

    // Update process states
    if (old_process && old_process->state == RUNNING) {
        old_process->state = READY;
    }

    //term_print("\n");
    //Logger::log(LogLevel::DEBUG, "Scheduling Thread %d (%s)", next_process->pid, next_process->is_kernel_mode ? "Kernel" : "User");

    uint32_t esp;

    Logger::serial_log("================================\n");
    
    asm volatile ("mov %%esp, %0" : "=r"(esp));
    Logger::serial_log("BEFORE KERNEL ESP: 0x%x \n", esp);

    if (old_process && old_process->pid != 0) {
        interruptFrame* old_ctx = &old_process->context;    
        *old_ctx = *interrupt_frame;

        //old_ctx->ss = 0x23;
        //old_ctx->cs = 0x1B;

        // Calculate stack usage
        uint32_t stack_usage = (((uint32_t)old_process->user_stack + STACK_SIZE) & ~0xF) - (uint32_t)old_ctx->esp;
        Logger::serial_log("Stack Usage: %d bytes\n", stack_usage);

        Logger::serial_log("Saved context for process PID %d \n", old_process->pid);
        print_interrupt_frame(&old_process->context); // Pass the context directly
        Logger::serial_log("Saved ESP: 0x%x \n", old_process->context.esp);
    }

    //asm volatile ("mov %%esp, %0" : "=r"(esp));
    //serial_log("AFTER KERNEL ESP: 0x%x \n", esp);

    // Load the new process's context
    if (next_process) {
        
        Logger::serial_log("Loading process PID %d \n", next_process->pid);
        print_interrupt_frame(&next_process->context);

        Logger::serial_log("Loading ESP: 0x%x \n", next_process->context.esp);

        //tss_set_stack(next_process->kernel_stack->top); //not needed for now
        //Logger::serial_log("TSS Stack set to 0x%x\n", next_process->kernel_stack->top);

        next_process->state = RUNNING;  // Mark the next process as RUNNING
        current_process = next_process;        

        load_context(&next_process->context); // Load the context of the next process
    }

    Logger::serial_log("scheduler end reached !?!?");
}

void terminate_current_process(int return_code) {
    if (!current_process || current_process->pid == 0) {  // Don't terminate idle process
        return;
    }

    // Mark as terminated first
    current_process->state = TERMINATED;
    
    // Free process resources
    if (current_process->kernel_stack) {
        StackManager::destroy_stack(current_process->kernel_stack);
        current_process->kernel_stack = nullptr;
    }
    if (current_process->user_stack) {
        StackManager::destroy_stack(current_process->user_stack);
        current_process->user_stack = nullptr;
    }
    if (current_process->fpu_state) {
        aligned_kfree(current_process->fpu_state);
        current_process->fpu_state = nullptr;
    }

    Logger::log(LogLevel::INFO, "Process PID %d terminated with code %d", current_process->pid, return_code);
    
    PCB* terminated = current_process;
    current_process = nullptr;

    // Force a reschedule
    schedule(nullptr);
    
    // Should never reach here
    while(1) { asm("hlt"); }
}