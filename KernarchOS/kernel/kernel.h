#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"
#include "terminal.h"
#include "isr.h"
#include "pic.h"
#include "memory.h"
#include "io.h"
#include "idt.h"
#include "paging.h"
#include "keyboard.h"
#include "multiboot.h"

//Dummy sleep to slow down the initialization process
void dummy_sleep(int mul){
    int a = 1000000 * mul;
    for (int i = 0; i < a; i++)
        asm volatile ("nop");
    return;
}

void terminalProcess();
void testThread(const char* name);
#endif // KERNEL_H