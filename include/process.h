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
    PROC_BLOCKED,
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
int    process_kill(uint32_t pid);
void   process_print_table(void);
pcb_t *process_get_by_pid(uint32_t pid);

#endif /* PROCESS_H */

/* Threads share address space with the kernel (no isolation yet — that's
   Stage 3's virtual memory). Functionally identical to create_process();
   named separately to match L10 terminology in ps/threads output. */
pcb_t *create_thread(void (*entry)(void), const char *name);
