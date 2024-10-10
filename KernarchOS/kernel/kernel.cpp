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

    //ACPI::initialize(); //Fix memory access

    //ACPI::instance()->scan_drives();

    multiboot_scan(mbd, magic);

    Logger::info("Kernel initialization complete");
    
    term_print("Welcome to KernarchOS!\n");

    //setup_and_switch_to_user_mode();

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

#include "user_program.h"

#define USER_SPACE_START 0x1000000  // 16MB
#define USER_SPACE_END 0x2000000 // 32MB
#define USER_STACK_SIZE 4096

void setup_and_switch_to_user_mode() {
    // Allocate memory for user program
    uint32_t user_program_size = 512; // Adjust as needed
    void* user_program_memory = (void*)USER_SPACE_START;


    // Copy user program to allocated memory
    //void* address = memcpy(user_program_memory, (void*)user_program, user_program_size);

    // Allocate user stack at the end of user space
    uint32_t user_stack_top = USER_SPACE_END - USER_STACK_SIZE;

    uint32_t esp;
    asm volatile("mov %%esp, %0" : "=r"(esp));
    tss_set_stack(KERNEL_DATA_SEG, esp); // Use the correct segment for kernel stack

    // Switch to user mode
    UserMode::switch_to_user_mode((uint32_t)user_program_memory, user_stack_top);
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
