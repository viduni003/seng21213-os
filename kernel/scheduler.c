#include "scheduler.h"
#include "process.h"
#include "pic.h"

extern void switch_context(uint32_t *old_esp_ptr, uint32_t new_esp);

#define MAX_TASKS MAX_PROCESSES

static pcb_t   *task_list[MAX_TASKS];
static int      task_count = 0;
static int      current_task = -1;
static volatile uint32_t tick_count = 0;

void scheduler_init(void) {
    task_count = 0;
    current_task = -1;
    tick_count = 0;
}

void scheduler_add_process(pcb_t *p) {
    if (task_count < MAX_TASKS) {
        task_list[task_count++] = p;
    }
}

pcb_t *scheduler_get_current(void) {
    if (current_task >= 0 && current_task < task_count) {
        return task_list[current_task];
    }
    return NULL;
}

void irq0_handler(void) {
    tick_count++;
    pic_send_eoi(0);

    if (task_count == 0) {
        return;
    }

    int prev = current_task;

    /* Find next runnable task in round-robin order */
    int next_task = current_task;
    int found = 0;
    for (int i = 0; i < task_count; i++) {
        next_task = (next_task + 1) % task_count;
        if (task_list[next_task]->state == PROC_READY ||
            task_list[next_task]->state == PROC_RUNNING) {
            found = 1;
            break;
        }
    }

    if (!found) {
        return;
    }

    if (prev == next_task) {
        return;
    }

    current_task = next_task;

    uint32_t *old_esp_ptr;
    uint32_t  new_esp;

    if (prev == -1) {
        static uint32_t dummy;
        old_esp_ptr = &dummy;
    } else {
        if (task_list[prev]->state == PROC_RUNNING) {
            task_list[prev]->state = PROC_READY;
        }
        old_esp_ptr = &task_list[prev]->esp;
    }

    task_list[current_task]->state = PROC_RUNNING;
    new_esp = task_list[current_task]->esp;

    switch_context(old_esp_ptr, new_esp);
}

uint32_t scheduler_get_ticks(void) {
    return tick_count;
}
