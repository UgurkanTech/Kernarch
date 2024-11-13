#ifndef PIT_H
#define PIT_H

#include "types.h"
#include "isr.h"

#define PIT_BASE_FREQUENCY 1193182

void pit_init(uint32_t frequency);
void pit_handler(interrupt_frame* interrupt_frame);
uint32_t pit_get_ticks();
uint32_t pit_get_milliseconds();

void pit_register_scheduler(void (*scheduler_func)(interrupt_frame* interrupt_frame));

#endif // PIT_H