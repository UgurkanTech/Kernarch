#ifndef PIC_H
#define PIC_H

#include "types.h"

// PIC ports
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

// End of interrupt command code
#define PIC_EOI 0x20

void pic_remap(int offset1, int offset2);
void pic_sendEOI(uint8_t irq);
void pic_init();

#endif // PIC_H