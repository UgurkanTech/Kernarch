#include "interrupts.h"
#include "process.h"
#include "keyboard.h"


// Parameter types and structure
union ParamValue {
    int32_t i;           // Integer value
    uint32_t u;          // Unsigned integer
    const char* str;     // String pointer
    void* ptr;           // Generic pointer
    bool b;              // Boolean value
    char c;
};

struct SyscallParams {
    uint32_t syscall_num;        // System call number
    uint32_t param_count;        // Number of parameters (max 4)
    ParamValue return_value;     // Return value
    ParamValue params[4];        // Parameter values
};

// System call handler
void syscall_handler(interrupt_frame* frame) {
    uint32_t syscall_num = frame->eax;
    SyscallParams* call_params = *(SyscallParams**)frame->esp;

    //Create struct from stack pointer
    switch (syscall_num) {
        case SYSCALL_PRINT:
            term_printf(call_params->params->str);
            break;
        case SYSCALL_READ:
            if(Keyboard::has_char())
                call_params->return_value.c = Keyboard::get_char();
            else
                call_params->return_value.c = '\0';
            break;
        case SYSCALL_CLEAR:
            term_clear();
            break;
        case SYSCALL_EXIT:
            term_printf("Process exited with code: %d\n", (int)call_params->params->u);
            break;
        default:
            term_printf("Failed syscall %d\n", syscall_num);
            break;
    }

}

// Low-level syscall function
static void _syscall(SyscallParams* params) {
    int result;
    uint32_t saved_eax;

    // Save EAX
    asm volatile ("movl %%eax, %0" : "=r" (saved_eax));

    // Push the entire struct onto the stack
    asm volatile (
        "pushl %0"            // Push the struct pointer
        :
        : "r" (params)
        : "memory"
    );

    // Make the system call
    asm volatile (
        "movl %0, %%eax\n"    // Set system call number
        "int %1\n"            // Trigger interrupt
        :
        : "r" (params->syscall_num), "i" (SYSCALL_INT)
        : "eax", "memory"
    );

    // Pop the struct pointer (cleanup)
    asm volatile ("addl $4, %esp");

    // Restore EAX
    asm volatile ("movl %0, %%eax" : : "r" (saved_eax));
}

// User space system call functions
void sys_printf(const char* format, ...) {
    if (!format) return;

    // Format the string
    char buffer[256];
    va_list args;
    va_start(args, format);
    vformat_string(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Create parameters on stack
    SyscallParams params = {
        .syscall_num = SYSCALL_PRINT,
        .param_count = 1,
        .params = {{ .str = buffer }}
    };

    _syscall(&params);
}

char sys_read() {
    SyscallParams params = {
        .syscall_num = SYSCALL_READ,
        .param_count = 0
    };

    _syscall(&params);

    return params.return_value.c;
}

void sys_clear() {
    SyscallParams params = {
        .syscall_num = SYSCALL_CLEAR,
        .param_count = 0
    };

    _syscall(&params);
}