/* =============================================================================
 * SENG21213-OS :: Counting Semaphore
 * File   : include/semaphore.h
 * Lecture: L10 §2 — Semaphores for producer-consumer synchronization
 * ============================================================================*/
#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "types.h"

typedef struct {
    volatile int32_t count;
} semaphore_t;

void sem_init(semaphore_t *s, int32_t initial_count);
void sem_wait(semaphore_t *s);   /* also called "P" or "down" */
void sem_signal(semaphore_t *s); /* also called "V" or "up" */

#endif /* SEMAPHORE_H */
