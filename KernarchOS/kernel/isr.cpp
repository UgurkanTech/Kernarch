#include "isr.h"


extern "C" void isr_handler(interrupt_frame frame) {
    // Handle the interrupt here. This is just a placeholder.
    switch (frame.int_no) {
        case 0:
            // Handle interrupt 0
            break;
        case 1:
            // Handle interrupt 1
            break;
        // ...
        default:
            // Handle unknown interrupt
            break;
    }

    // If the interrupt number is greater than 31, it's an IRQ, so send an EOI
    if (frame.int_no > 31) {
        pic_sendEOI(frame.int_no - 32);
    }
}

extern "C" void isr_install() {
    // Set up the IDT entries for the ISRs
    for (uint8_t i = 0; i < 32; i++) {
        idt_set_gate(i, (uint32_t)isr_handler, 0x08, 0x8E);
    }

    // Flush the IDT
    idt_flush((uint32_t)&idt_ptr);
}