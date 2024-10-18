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

void terminalProcess();

#endif // KERNEL_H