#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

void irq0_handler(void);
void scheduler_init(void);
void scheduler_add_process(pcb_t *p);
pcb_t *scheduler_get_current(void);
uint32_t scheduler_get_ticks(void);

#endif
