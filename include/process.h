/* =============================================================================
 * SENG21213-OS :: Process Control Block
 * File   : include/process.h
 * Lecture: L09 §2 — Process creation and the PCB
 * ============================================================================*/
#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

#define MAX_PROCESSES   8
#define PROCESS_STACK_SIZE  4096

typedef enum {
    PROC_UNUSED,     /* slot not in use */
    PROC_READY,
    PROC_RUNNING,
    PROC_TERMINATED
} proc_state_t;

typedef struct pcb {
    uint32_t      pid;
    proc_state_t  state;
    uint32_t      esp;                        /* saved stack pointer */
    uint8_t       stack[PROCESS_STACK_SIZE];   /* this process's own stack */
    char          name[16];
} pcb_t;

void   process_init(void);
pcb_t *create_process(void (*entry)(void), const char *name);

#endif /* PROCESS_H */
