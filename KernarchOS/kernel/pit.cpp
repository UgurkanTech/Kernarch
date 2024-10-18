#include "pit.h"
#include "io.h"
#include "terminal.h"

//Warning: pit will overflow after about 49.7 days at 1000 Hz
volatile uint32_t pit_ticks = 0;
uint32_t pit_frequency = 0;
void (*scheduler_callback)() = nullptr;

void pit_init(uint32_t frequency) {
    pit_frequency = frequency;
    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;
    
    outb(0x43, 0x36);  // Command byte: Channel 0, lobyte/hibyte, square wave
    outb(0x40, divisor & 0xFF);         // Low byte
    outb(0x40, (divisor >> 8) & 0xFF);  // High byte
}

void pit_handler() {
    pit_ticks++;

    if (pit_ticks % 10 == 0 && scheduler_callback) {
        scheduler_callback();
    }
}

uint32_t pit_get_ticks() {
    return pit_ticks;
}

uint32_t pit_get_milliseconds() {
    return (pit_ticks * 1000) / pit_frequency;
}

void pit_sleep(uint32_t milliseconds) {
    uint32_t start = pit_get_milliseconds();
    while (pit_get_milliseconds() - start < milliseconds) {
        asm volatile("hlt");  // Halt the CPU until the next interrupt
    }
}

void pit_register_scheduler(void (*scheduler_func)()) {
    scheduler_callback = scheduler_func;
}