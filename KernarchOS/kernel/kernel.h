#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>
#include <stdint.h>
#include "terminal.h"
#include "isr.h"
#include "pic.h"
#include "memory.h"
#include "io.h"
#include "idt.h"
#include "paging.h"
#include "keyboard.h"

void user_mode_entry();
void switch_to_user_mode();

#endif // KERNEL_H