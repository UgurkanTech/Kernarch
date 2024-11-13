#include "kernel.h"
#include "terminal.h"
#include "idt.h"
#include "isr.h"
#include "pit.h"
#include "pic.h"
#include "memory.h"
#include "paging.h"
#include "keyboard.h"
#include "logger.h"
#include "commands.h"
#include "cstring.h"
#include "tss.h"
#include "process.h"

//Temporary stack allocator which will be replaced later..
uint32_t allocate_stack(uint32_t size) {
    void* stack = kmalloc(size); // Allocate 4KB of stack space
    if (!stack) {
        Logger::log(LogLevel::ERROR, "Failed to allocate stack"); // Log allocation failure
        return 0;
    }
    uint32_t stack_top = (uint32_t)stack + size; // Set stack top to end of allocated space
    stack_top &= ~0xF;  // Align to 16 bytes
    return stack_top;
}

//Dummy sleep until sleep system call is implemented
void dummy_sleep(int mul){
    int a = 1000000 * mul;
    for (int i = 0; i < a; i++)
        asm volatile ("nop");
    return;
}

// Main Kernel Entry
extern "C" void kernel_main(multiboot_info_t* mbd, unsigned int magic) {
    term_init();
    Logger::setLogLevel(DEBUG);

    //Command struct
    struct Command {
        void (*func)(void*); // Function pointer taking a void pointer for flexibility
        void* arg1;          // First argument
        void* arg2;          // Second argument (if needed)
        const char* log_msg; // Log message for each command
    };

    // Array of commands with their corresponding arguments and log messages
    Command commands[] = {
        { (void (*)(void*))init_gdt, NULL, NULL, "GDT" },
        { (void (*)(void*))init_tss, NULL, NULL, "TSS" },
        { (void (*)(void*))idt_init, NULL, NULL, "IDT" },
        { (void (*)(void*))isr_install, NULL, NULL, "ISRs" },
        { (void (*)(void*))pic_init, NULL, NULL, "PIC" },
        { (void (*)(void*))init_paging, NULL, NULL, "Paging" },
        { (void (*)(void*))multiboot_scan, mbd, (void*)magic, "Multiboot" }, // Pass mbd and magic as arguments
        { (void (*)(void*))init_memory, NULL, NULL, "Memory" },
        { (void (*)(void*))pit_init, (void*)1000, NULL, "PIT" }, // Pass 1000 as the first argument
        { (void (*)(void*))Commands::initialize, NULL, NULL, "Commands" },
        { (void (*)(void*))init_processes, NULL, NULL, "Processes" },
    };

    Logger::info("Initializing kernel...");
        // Execute each command with a single dummy_sleep
    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        Logger::log(LogLevel::INFO ,"Initializing %s...", commands[i].log_msg); // Log the command message before execution
        dummy_sleep(100);
        if (commands[i].arg1 && commands[i].arg2) {
            ((void (*)(void*, void*))commands[i].func)(commands[i].arg1, commands[i].arg2); // Call function with two arguments
        } else if (commands[i].arg1) {
            ((void (*)(void*))commands[i].func)(commands[i].arg1); // Call function with one argument
        } else {
            ((void (*)(void*))commands[i].func)(NULL); // Call function without arguments
        }
        dummy_sleep(100); // Sleep after each command
        Logger::log(LogLevel::DEBUG, "DONE! (%d/%d)", i+1, sizeof(commands) / sizeof(commands[0])); // Log the command execution
    }
    // Create the terminal process
    create_process(terminalProcess, true);
    // Create the terminal process
    create_process(terminalProcess2, false);

    dummy_sleep(100);
    tss_set_stack(allocate_stack(4096));

    asm volatile ("sti");
    Logger::info("Interrupts enabled");
    dummy_sleep(100);
    Logger::info("Kernel initialization complete");
    term_print("Welcome to KernarchOS!\n");
    dummy_sleep(100);

    schedule(nullptr); //Call schedular which will not return.

    for(;;) asm("hlt");
}

//Temporary test processes
void terminalProcess() {
    term_print("Krnl Proc> ");
    int a = 99;
    while (true) {
        a++;
        dummy_sleep(50);
        
        term_print_int(a);
        term_print_colored(".", VGA_LIGHT_RED);
    }
    term_print_int(a);
}

void terminalProcess2() {
    term_print("User Proc> ");
    int a = 0;
    while (true) {
        a++;
        dummy_sleep(50);

        term_print_int(a);
        term_print_colored(",", VGA_LIGHT_GRAY);
    }
    term_print_int(a);
}
