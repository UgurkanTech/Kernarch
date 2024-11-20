#include "isr.h"
#include "idt.h"
#include "pic.h"
#include "pit.h"
#include "terminal.h"
#include "keyboard.h"
#include "interrupts.h"
#include "kernel_config.h"
#include "io.h"
#include "string_utils.h"
#include "types.h"
#include "process.h"
#include "memory.h"

static interrupt_handler_t handlers[256][MAX_HANDLERS_PER_INTERRUPT];
static uint8_t handler_counts[256];

extern "C" void isr_install() {
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, (uint32_t)isr_stub_table[i], 0x08, 0xEE);
    }
    //idt_set_gate(0x80, (uint32_t)isr_stub_table[128], 0x08, 0xEE);  // User mode accessible

    for (int i = 0; i < 256; i++) {
        handler_counts[i] = 0;
        for (int j = 0; j < MAX_HANDLERS_PER_INTERRUPT; j++) {
            handlers[i][j] = NULL;
        }
    }

    register_interrupt_handler(0x80, syscall_handler);
}

static interrupt_type_t get_interrupt_type(uint8_t vector) {
    if (vector < HARDWARE_INT_BASE) return INTERRUPT_TYPE_EXCEPTION;
    if (vector < SOFTWARE_INT_BASE) return INTERRUPT_TYPE_HARDWARE;
    if (vector < 256) return INTERRUPT_TYPE_SOFTWARE;
    return INTERRUPT_TYPE_UNKNOWN;
}

void save_fpu_state(PCB* pcb) {
    if (!pcb || !pcb->fpu_state) return;
    __asm__ volatile ("fxsave (%0)" : : "r"(pcb->fpu_state) : "memory");
}

void restore_fpu_state(PCB* pcb) {
    if (!pcb || !pcb->fpu_state) return;
    __asm__ volatile ("fxrstor (%0)" : : "r"(pcb->fpu_state) : "memory");
}

// Initialize FPU state for a process
void init_fpu_state(PCB* pcb) {
    if (!pcb || pcb->fpu_state) return;  // Already initialized
    
    // Allocate aligned memory for FPU state
    pcb->fpu_state = (uint8_t*)aligned_kmalloc(16, 512);
    if (!pcb->fpu_state) {
        term_printf("Failed to allocate FPU state memory\n");
        return;
    }
    // Clear the FPU state memory
    memset(pcb->fpu_state, 0, 512);
    
    term_printf("FPU initialized for process %d\n", pcb->pid);
}

PCB* last_fpu_owner = nullptr;

static void handle_exception(uint8_t vector, interrupt_frame* frame) {
    switch (vector) 
    {
    case EXC_DEVICE_NOT_AVAILABLE:
        if (!current_process) {
            term_printf("No current process in FPU handler!\n");
            break;
        }
        // Clear TS flag first
        __asm__ volatile ("clts");
        // Initialize process FPU state if needed
        if (!current_process->fpu_state) {
            init_fpu_state(current_process);
        }
        // Only save and restore if the last FPU owner is different
        if (last_fpu_owner != current_process) {
            // Save previous FPU state if necessary
            if (last_fpu_owner != nullptr) {
                save_fpu_state(last_fpu_owner);
            }

            // Restore current process FPU state
            restore_fpu_state(current_process);
            term_printf("FPU state switched from process %d to %d\n", 
                       last_fpu_owner ? last_fpu_owner->pid : -1, 
                       current_process->pid);
            
            // Update the last FPU owner
            last_fpu_owner = current_process;
        }
        break;
    default:
        print_interrupt_frame(frame, vector);
        term_printf("Kernel Panic! - %s", exception_messages[vector]);
        while(1) {} // Halt on exception
        break;
    }
}

extern "C" void isr_handler(uint8_t vector, interrupt_frame frame) {
    interrupt_type_t type = get_interrupt_type(vector);
    switch (type) {
        case INTERRUPT_TYPE_EXCEPTION:
            handle_exception(vector, &frame);
            break;

        case INTERRUPT_TYPE_HARDWARE: {
            uint8_t irq = vector - HARDWARE_INT_BASE;
            // Execute all registered handlers
            for (int i = 0; i < handler_counts[vector]; i++) {
                if (handlers[vector][i]) {
                    handlers[vector][i](&frame);
                }
            }
            pic_sendEOI(irq);
            break;
        }

        case INTERRUPT_TYPE_SOFTWARE:
            // Execute all registered handlers
            for (int i = 0; i < handler_counts[vector]; i++) {
                if (handlers[vector][i]) {
                    handlers[vector][i](&frame);
                }
            }
            break;

        case INTERRUPT_TYPE_UNKNOWN:
            term_print("Received unexpected interrupt: ");
            print_interrupt_frame(&frame, vector);
            break;
    }
}

bool register_interrupt_handler(uint8_t vector, interrupt_handler_t handler) {
    if (!handler || handler_counts[vector] >= MAX_HANDLERS_PER_INTERRUPT) {
        return false;
    }

    // Check if handler is already registered
    for (int i = 0; i < handler_counts[vector]; i++) {
        if (handlers[vector][i] == handler) {
            return false; // Handler already registered
        }
    }

    handlers[vector][handler_counts[vector]++] = handler;
    return true;
}

bool unregister_interrupt_handler(uint8_t vector, interrupt_handler_t handler) {
    if (!handler || handler_counts[vector] == 0) {
        return false;
    }

    // Find and remove the handler
    bool found = false;
    for (int i = 0; i < handler_counts[vector]; i++) {
        if (handlers[vector][i] == handler) {
            found = true;
            // Shift remaining handlers down
            for (int j = i; j < handler_counts[vector] - 1; j++) {
                handlers[vector][j] = handlers[vector][j + 1];
            }
            handler_counts[vector]--;
            handlers[vector][handler_counts[vector]] = NULL;
            break;
        }
    }
    return found;
}