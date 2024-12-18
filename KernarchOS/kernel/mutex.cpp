#include "mutex.h"
#include "terminal.h"
#include "interrupts.h"

// Lock the mutex (spinlock until successful)
void Mutex::lock() {
    while (__atomic_test_and_set(&lockFlag, __ATOMIC_ACQUIRE)) {
        sys_schedule();
    }
}

// Unlock the mutex
void Mutex::unlock() {
    __atomic_clear(&lockFlag, __ATOMIC_RELEASE);
}
