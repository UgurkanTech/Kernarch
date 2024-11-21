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
#include "interrupts.h"
#include "stack.h"
#include "thread.h"


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
        { (void (*)(void*))multiboot_scan, mbd, (void*)magic, "Multiboot" },
        { (void (*)(void*))init_memory, NULL, NULL, "Memory" },
        { (void (*)(void*))pit_init, (void*)1000, NULL, "PIT" },
        { (void (*)(void*))Commands::initialize, NULL, NULL, "Commands" },
        { (void (*)(void*))init_processes, NULL, NULL, "Processes" },
        { (void (*)(void*))Keyboard::init, NULL, NULL, "Keyboard" },
    };

    Logger::info("Initializing kernel...");
        // Execute each command with a single dummy_sleep
    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        Logger::log(LogLevel::INFO ,"Initializing %s...", commands[i].log_msg); // Log the command message before execution
        dummy_sleep(10);
        if (commands[i].arg1 && commands[i].arg2) {
            ((void (*)(void*, void*))commands[i].func)(commands[i].arg1, commands[i].arg2); // Call function with two arguments
        } else if (commands[i].arg1) {
            ((void (*)(void*))commands[i].func)(commands[i].arg1); // Call function with one argument
        } else {
            ((void (*)(void*))commands[i].func)(NULL); // Call function without arguments
        }
        dummy_sleep(10); // Sleep after each command
        Logger::log(LogLevel::DEBUG, "DONE! (%d/%d)", i+1, sizeof(commands) / sizeof(commands[0])); // Log the command execution
    }
    // Create the terminal thread and test threads
    ThreadManager::create_thread(terminalProcess);
    ThreadManager::create_thread(testThread, "Test1");
    ThreadManager::create_thread(testThread, "Test2");

    dummy_sleep(100);
    tss_set_stack(StackManager::allocate_stack(8192)->top);
    dummy_sleep(100);
    Logger::info("Kernel initialization complete");
    term_print("Welcome to KernarchOS!\n");
    dummy_sleep(100);

    schedule(nullptr); //Call schedular which will not return.

    for(;;) asm("hlt");
}

void terminalProcess(){
    while (true) {
        char c = sys_read();
        if (c != '\0')
            term_input(c);
        
        thread_sleep(10);
    }
}
//Temporary test processes
void testThread(const char* name) {
    sys_printf("&eStarting %s Process Counting =>\n", name);
    int a = 0;
    while (true) {
        a += 1;
        thread_sleep(300);
        uint32_t esp = 0;
        asm volatile ("mov %%esp, %0" : "=r"(esp));
        if(a % 10 == 0)
            return;
        sys_printf("%s - &3Stack: 0x%x, &7num: %d   \n", name, esp, a);
    }
    sys_printf("%d", a);
}