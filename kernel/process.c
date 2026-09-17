#include "process.h"

static pcb_t   process_table[MAX_PROCESSES];
static uint32_t next_pid = 1;

extern void task_start_trampoline(void);

static void k_strcpy(char *dst, const char *src, size_t max) {
    size_t i = 0;
    while (src[i] && i < max - 1) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

void process_init(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].state = PROC_UNUSED;
    }
    next_pid = 1;
}

pcb_t *create_process(void (*entry)(void), const char *name) {
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROC_UNUSED) { slot = i; break; }
    }
    if (slot == -1) return NULL;

    pcb_t *p = &process_table[slot];
    p->pid = next_pid++;
    p->state = PROC_READY;
    k_strcpy(p->name, name, sizeof(p->name));

    uint32_t *sp = (uint32_t *)(p->stack + PROCESS_STACK_SIZE);

    *(--sp) = (uint32_t)task_start_trampoline;

    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = (uint32_t)entry;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;

    p->esp = (uint32_t)sp;

    return p;
}
