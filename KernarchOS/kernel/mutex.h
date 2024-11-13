#ifndef MUTEX_H
#define MUTEX_H

#include "types.h"

class Mutex {
public:
    void lock();
    void unlock();

    volatile uint32_t lockFlag; // 0 means unlocked, 1 means locked
};

#endif // MUTEX_H
