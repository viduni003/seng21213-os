/* =============================================================================
 * SENG21213-OS :: Mutex
 * File   : include/mutex.h
 * Lecture: L10 §1 — Mutual exclusion via interrupt disable (uniprocessor)
 * ============================================================================*/
#ifndef MUTEX_H
#define MUTEX_H

#include "types.h"

typedef struct {
    volatile uint32_t locked;
} mutex_t;

void mutex_init(mutex_t *m);
void mutex_lock(mutex_t *m);
void mutex_unlock(mutex_t *m);

#endif /* MUTEX_H */
