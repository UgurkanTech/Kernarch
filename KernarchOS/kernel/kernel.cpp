#include "kernel.h"
#include "terminal.h"
#include "idt.h"
#include "isr.h"
#include "pit.h"
#include "pic.h"
#include "memory.h"
#include "paging.h"
#include "keyboard.h"

extern "C" void kernel_main() {
    term_init();
    term_print("Initializing kernel...\n");

    idt_init();
    term_print("IDT initialized\n");

    isr_install();
    term_print("ISRs installed\n");

    term_print("About to initialize PIC...\n");
    pic_init();
    term_print("PIC initialized\n");

    term_print("About to initialize paging...\n");
    init_paging();
    term_print("Paging initialized\n");



    term_print("About to initialize memory...\n");
    init_memory();
    term_print("Memory initialized\n");

    print_heap_info();

    term_print("About to initialize PIT...\n");
    pit_init(1000); // 1ms tick
    term_print("PIT initialized\n");
    // Enable interrupts
    term_print("About to enable interrupts...\n");
    asm volatile ("sti");
    term_print("Interrupts enabled\n");

    term_print("Kernel initialization complete\n");

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