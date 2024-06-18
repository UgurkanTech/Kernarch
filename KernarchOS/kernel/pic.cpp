#include "pic.h"
#include "io.h"

void pic_remap(int offset1, int offset2) {
    unsigned char a1, a2;

    a1 = inb(PIC1_DATA); // save masks
    a2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, 0x11); // starts the initialization sequence
    outb(PIC2_COMMAND, 0x11);

    outb(PIC1_DATA, offset1); // define the PIC vectors
    outb(PIC2_DATA, offset2);

    outb(PIC1_DATA, 4); // continue initialization sequence
    outb(PIC2_DATA, 2);

    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    outb(PIC1_DATA, a1); // restore saved masks
    outb(PIC2_DATA, a2);
}

void pic_sendEOI(uint8_t irq) {
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);

    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_init() {
    pic_remap(0x20, 0x28);
}