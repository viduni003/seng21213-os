#include "mutex.h"

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

void mutex_init(mutex_t *m) {
    m->locked = 0;
}

void mutex_lock(mutex_t *m) {
    while (1) {
        uint32_t flags = save_and_disable_interrupts();
        if (!m->locked) {
            m->locked = 1;
            restore_interrupts(flags);
            return;
        }
        restore_interrupts(flags);
    }
}

void mutex_unlock(mutex_t *m) {
    uint32_t flags = save_and_disable_interrupts();
    m->locked = 0;
    restore_interrupts(flags);
}
