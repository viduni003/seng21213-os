/* =============================================================================
 * SENG21213-OS :: Semaphore implementation
 * File   : kernel/semaphore.c
 * Lecture: L10 §2
 *
 * Simplified busy-wait implementation (no true blocking/sleep queue yet -
 * that would require deeper scheduler integration). wait() spins while
 * count <= 0, yielding CPU time to other threads via the timer preemption
 * that's already happening constantly in this kernel.
 * ============================================================================*/
#include "semaphore.h"

static inline uint32_t save_and_disable_interrupts(void) {
    uint32_t flags;
    __asm__ __volatile__(
        "pushf\n\t"
        "pop %0\n\t"
        "cli"
        : "=r"(flags)
    );
    return flags;
}

static inline void restore_interrupts(uint32_t flags) {
    __asm__ __volatile__(
        "push %0\n\t"
        "popf"
        :
        : "r"(flags)
    );
}

void sem_init(semaphore_t *s, int32_t initial_count) {
    s->count = initial_count;
}

void sem_wait(semaphore_t *s) {
    while (1) {
        uint32_t flags = save_and_disable_interrupts();
        if (s->count > 0) {
            s->count--;
            restore_interrupts(flags);
            return;
        }
        restore_interrupts(flags);
        /* busy-wait; scheduler preempts us here on the next tick */
    }
}

void sem_signal(semaphore_t *s) {
    uint32_t flags = save_and_disable_interrupts();
    s->count++;
    restore_interrupts(flags);
}
