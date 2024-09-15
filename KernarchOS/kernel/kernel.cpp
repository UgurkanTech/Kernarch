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


extern "C" void kernel_main() {

    term_init();

    Logger::init();
    Logger::set_log_level(INFO);

    Logger::info("Initializing kernel...");

    idt_init();

    Logger::info("IDT initialized");

    isr_install();
    
    Logger::info("ISRs installed");

    pic_init();
    
    Logger::info("PIC initialized");

    init_paging();

    Logger::info("Paging initialized");

    init_memory();

    Logger::info("Memory initialized");

    set_text_color(VGA_YELLOW);
    print_memory_info();
    set_text_color(VGA_WHITE);

    pit_init(1000); // 1ms tick
    
    Logger::info("PIT initialized");

    asm volatile ("sti");
    
    Logger::info("Interrupts enabled");

    Commands::initialize();

    ACPI::initialize();

    ACPI::instance()->scan_drives();

    Logger::info("Kernel initialization complete");


    term_print("Welcome to KernarchOS!\n");
    term_print("> ");
    // Main kernel loop
    while (true) {
        char c = Keyboard::get_char();
        term_input(c);
    }
    // Enter an infinite loop
    for (;;) {
        asm volatile ("hlt");
    }
}