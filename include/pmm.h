/* =============================================================================
 * SENG21213-OS :: Physical Memory Manager
 * File   : include/pmm.h
 * Lecture: L11 §1-2 — E820 parsing, bitmap frame allocator
 * ============================================================================*/
#ifndef PMM_H
#define PMM_H

#include "types.h"

#define FRAME_SIZE 4096

void     pmm_init(void);
uint32_t pmm_alloc_frame(void);   /* returns physical address, or 0 if OOM */
void     pmm_free_frame(uint32_t paddr);

uint32_t pmm_total_frames(void);
uint32_t pmm_used_frames(void);
uint32_t pmm_free_frames(void);

#endif /* PMM_H */
