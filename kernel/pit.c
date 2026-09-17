/* =============================================================================
 * SENG21213-OS :: PIT implementation
 * File   : kernel/pit.c
 * Lecture: L09 §2
 * ============================================================================*/
#include "pit.h"

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

#define PIT_CH0_DATA 0x40
#define PIT_CMD      0x43
#define PIT_BASE_HZ  1193182

void pit_init(uint32_t frequency_hz) {
    uint32_t divisor = PIT_BASE_HZ / frequency_hz;

    outb(PIT_CMD, 0x36); /* channel 0, lo/hi byte, mode 3 (square wave) */
    outb(PIT_CH0_DATA, (uint8_t)(divisor & 0xFF));
    outb(PIT_CH0_DATA, (uint8_t)((divisor >> 8) & 0xFF));
}
