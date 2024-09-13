#include "math64.h"

uint64_t add64(uint64_t a, uint64_t b) {
    uint32_t low = (uint32_t)a + (uint32_t)b;
    uint32_t high = (a >> 32) + (b >> 32) + (low < (uint32_t)a);
    return ((uint64_t)high << 32) | low;
}

uint64_t sub64(uint64_t a, uint64_t b) {
    uint32_t low = (uint32_t)a - (uint32_t)b;
    uint32_t high = (a >> 32) - (b >> 32) - (low > (uint32_t)a);
    return ((uint64_t)high << 32) | low;
}

uint64_t mul64(uint64_t a, uint64_t b) {
    uint32_t a0 = (uint32_t)a, a1 = a >> 32;
    uint32_t b0 = (uint32_t)b, b1 = b >> 32;
    uint64_t p0 = (uint64_t)a0 * b0;
    uint64_t p1 = (uint64_t)a1 * b0 + (p0 >> 32);
    uint64_t p2 = (uint64_t)a0 * b1 + (uint32_t)p1;
    return ((p2 + ((uint64_t)a1 * b1)) << 32) + (uint32_t)p0;
}

uint64_t div64(uint64_t num, uint64_t den) {
    if (den == 0) return 0xFFFFFFFFFFFFFFFF; // Divide by zero error

    uint64_t quot = 0, qbit = 1;
    while ((int64_t)den >= 0) {
        den <<= 1;
        qbit <<= 1;
    }
    while (qbit) {
        if (den <= num) {
            num -= den;
            quot += qbit;
        }
        den >>= 1;
        qbit >>= 1;
    }
    return quot;
}

uint64_t mod64(uint64_t num, uint64_t den) {
    if (den == 0) return 0; // Divide by zero error

    uint64_t qbit = 1;
    while ((int64_t)den >= 0) {
        den <<= 1;
        qbit <<= 1;
    }
    while (qbit) {
        if (den <= num) {
            num -= den;
        }
        den >>= 1;
        qbit >>= 1;
    }
    return num; // Remainder
}