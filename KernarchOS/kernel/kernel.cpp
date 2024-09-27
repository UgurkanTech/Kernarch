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
#include "user_mode.h"
#include "tss.h"
#include "disk.h"
#include "fat32.h"



extern "C" void kernel_main() {

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
    while (true) {
        char c = Keyboard::get_char();
        term_input(c);
    }

    // Enter an infinite loop
    for (;;) {
        asm volatile ("hlt");
    }
}


void initDisks() {
    // Scan for drives
    ACPI::instance()->scan_drives();

    // Initialize Disk
    Disk* disk = Disk::instance();
    if (!disk->initialize()) {
        Logger::error("Failed to initialize Disk");
        return;
    }

    // Format the disk only if it's not already formatted
    if (!disk->is_formatted()) {
        if (!disk->format_as_fat32("KERNARCHOS")) {
            Logger::error("Failed to format disk");
            return; // Early exit on failure
        }
        Logger::info("Disk formatted successfully");
    } else {
        Logger::info("Disk is already formatted");
    }

    // Initialize FAT32
    FAT32* fat32 = FAT32::instance();
    if (!fat32->initialize(disk)) {
        Logger::error("Failed to initialize FAT32");
    }
}

// This function will be copied to user space
__attribute__((section(".text"))) void user_mode_entry() {
    asm volatile("int $0x80");  // Trigger a system call
    volatile uint32_t test_var = 0;
    while(1) {
        test_var++;
        if (test_var % 1000000 == 0) {
            asm volatile ("int $0x80");
        }
    }
}



