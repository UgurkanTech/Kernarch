#include "kernel.h"

extern "C" void kernel_main()
{
	//term_init();
	term_print("\n");
    term_print("\n");
	term_print("Welcome to the kernel in Cpp!\n");

	idt_init();

	pic_init();

	isr_install();

	// Enable hardware interrupts
    asm volatile ("sti");

	term_print("Hardware interrupts enabled\n");

	// Enter an infinite loop
    for (;;) {
        asm volatile ("hlt");
    }
	
}