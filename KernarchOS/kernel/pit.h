#ifndef PIT_H
#define PIT_H

#include <stdint.h>

#define PIT_BASE_FREQUENCY 1193182

void pit_init(uint32_t frequency);
void pit_handler();
uint32_t pit_get_ticks();
uint32_t pit_get_milliseconds();
void pit_sleep(uint32_t milliseconds);

#endif // PIT_H