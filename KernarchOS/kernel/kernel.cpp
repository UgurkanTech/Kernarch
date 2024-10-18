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
#include "acpi.h"
#include "cstring.h"
#include "tss.h"
#include "disk.h"
#include "fat32.h"
#include "process.h"


extern "C" void kernel_main(multiboot_info_t* mbd, unsigned int magic) {

    term_init();

    Logger::init();
    Logger::set_log_level(INFO);

    Logger::info("Initializing kernel...");

    init_gdt();

    Logger::info("GDT initialized");

    init_tss((uint32_t)&stack_top); 

    Logger::info("TSS initialized");

    idt_init();

    Logger::info("IDT initialized");

    isr_install();
    
    Logger::info("ISRs installed");

    pic_init();
    
    Logger::info("PIC initialized");

    init_paging();

    Logger::info("Paging initialized");

    multiboot_scan(mbd, magic);
    init_memory();

    Logger::info("Memory initialized");

    set_text_color(VGA_YELLOW);
    print_memory_info();
    set_text_color(VGA_WHITE);

    pit_init(1);
    
    Logger::info("PIT initialized");


    Commands::initialize();

    init_processes();
    Logger::info("Process management system initialized");

    // Create the terminal process
    PCB* terminal_process = create_process(terminalProcess, true);
    if (terminal_process) {
        Logger::info("Terminal process created with PID %d", terminal_process->pid);
    } else {
        Logger::error("Failed to create terminal process");
    }

    asm volatile ("sti");
    Logger::info("Interrupts enabled");

    Logger::info("Kernel initialization complete");
    term_print("Welcome to KernarchOS!\n");

    schedule();
    

    // Enter an infinite loop
    for (;;) {
        asm volatile ("hlt");
    }
}


void terminalProcess() {
    term_print("user> ");
    while (true) {
        char c = Keyboard::get_char();
        term_input(c);
    }
}

