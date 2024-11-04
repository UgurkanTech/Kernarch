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

extern "C" void kernel_main(multiboot_info_t* mbd, unsigned int magic) {
    term_init();

    Logger::init();
    Logger::set_log_level(INFO);

    Logger::info("Initializing kernel...");

    init_gdt();
    Logger::info("GDT initialized");

    init_tss(); 
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

    pit_init(1000);
    Logger::info("PIT initialized");

    Commands::initialize();
    Logger::info("Commands are loaded");

    init_processes();
    Logger::info("Process management system initialized");

    // Create the terminal process
    PCB* terminal_process = create_process(terminalProcess, true);
    if (terminal_process) {
        Logger::info("Terminal process created with PID %d", terminal_process->pid);
    } else {
        Logger::error("Failed to create terminal process");
    }

    // Create the terminal process
    PCB* terminal_process2 = create_process(terminalProcess2, false);
    if (terminal_process2) {
        Logger::info("Terminal2 process created with PID %d", terminal_process2->pid);
    } else {
        Logger::error("Failed to create terminal process2");
    }

    tss_set_stack(allocate_stack(4096));

    asm volatile ("sti");
    Logger::info("Interrupts enabled");

    Logger::info("Kernel initialization complete");
    term_print("Welcome to KernarchOS!\n");

    schedule(nullptr);

    for(;;) asm("hlt");
}

void terminalProcess() {
    term_print("Krnl Proc> ");
    int a = 99;
    while (true) {
        a++;
       for (int i = 0; i < 35000000; i++)
            asm volatile ("nop");

        term_print_int(a);
        term_print(".");
    }
    term_print_int(a);
}

void terminalProcess2() {
    term_print("User Proc> ");
    int a = 0;
    while (true) {
        a++;
        for (int i = 0; i < 35000000; i++)
            asm volatile ("nop");

        term_print_int(a);
        term_print(",");
    }
    term_print_int(a);
}
