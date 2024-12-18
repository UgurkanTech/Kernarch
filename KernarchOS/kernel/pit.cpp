#include "pit.h"
#include "io.h"
#include "terminal.h"
#include "logger.h"
#include "isr.h"
#include "process.h"

//Warning: pit will overflow after about 49.7 days at 1000 Hz
volatile uint32_t pit_ticks = 0;
uint32_t pit_frequency = 0;
void (*scheduler_callback)(interrupt_frame* interrupt_frame) = nullptr;

void pit_init(uint32_t frequency) {
    pit_frequency = frequency;
    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;
    
    outb(0x43, 0x36);  // Command byte: Channel 0, lobyte/hibyte, square wave
    outb(0x40, divisor & 0xFF);         // Low byte
    outb(0x40, (divisor >> 8) & 0xFF);  // High byte

    register_interrupt_handler(INT_TIMER, pit_handler);
}

void pit_handler(interrupt_frame* interrupt_frame) {
    pit_ticks++;

    if (pit_ticks % 10 == 0 && scheduler_callback) {
        outb(0x20, 0x20); // Send EOI to the master PIC
        scheduler_callback(interrupt_frame);
    }
}

uint32_t pit_get_ticks() {
    return pit_ticks;
}

uint32_t get_current_time_ms() {
    return (pit_ticks * 1000) / pit_frequency;
}


void pit_register_scheduler(void (*scheduler_func)(interrupt_frame* interrupt_frame)) {
    scheduler_callback = scheduler_func;
}