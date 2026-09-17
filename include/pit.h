/* =============================================================================
 * SENG21213-OS :: PIT (Programmable Interval Timer, i8253)
 * File   : include/pit.h
 * Lecture: L09 §2 — Timer interrupt drives preemptive scheduling
 * ============================================================================*/
#ifndef PIT_H
#define PIT_H

#include "types.h"

void pit_init(uint32_t frequency_hz);

#endif /* PIT_H */
