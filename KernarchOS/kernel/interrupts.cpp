#include "interrupts.h"
#include "process.h"

//unused
void handle_software_interrupt(uint8_t int_no, interrupt_frame* frame) {
    switch (int_no) {
        case INT_OUT_OF_MEMORY:
            term_print("Out of memory! Kernel will terminate some processes now.\n");
            break;
        case Sleep:
            for (int i = 0; i < 35000000; i++)
                asm volatile ("nop");
            break;
        default:
            term_print("Unknown software interrupt!\n");
            break;
    }
}


void yield_interrupt(interrupt_frame* frame){
    Logger::serial_log("Yield interrupt triggered!\n");
    schedule(frame);
}


// System call handler
void syscall_handler(interrupt_frame* frame) {
    int syscall_num = frame->eax;   // Directly read syscall number from frame
    const char* format = (const char*) frame->ebx;  // Directly read format string pointer from frame

    switch (syscall_num) {
        case SYSCALL_PRINT:
            term_printf(format);  // Call kernel `term_printf` function
            break;
        default:
            term_printf("Failed syscall %d\n", syscall_num);
            break;
    }
}

// User-mode printf system call
void sys_printf(const char* format, ...) {
    if (!format) return;

    char buffer[256];
    va_list args;
    va_start(args, format);
    
    int length = vformat_string(buffer, sizeof(buffer), format, args);
    va_end(args);

    asm volatile (
        "movl %0, %%eax\n"       // System call number in eax
        "movl %1, %%ebx\n"       // Format string in ebx
        "int %2"                 // Trigger interrupt
        :
        : "i" (SYSCALL_PRINT), "r" (buffer), "i" (SYSCALL_INT)
        : "eax", "ebx"
    );
}