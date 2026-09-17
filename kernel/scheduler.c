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

void irq0_handler(void) {
    tick_count++;
    pic_send_eoi(0);

    if (task_count == 0) {
        return;
    }

    int prev = current_task;
    current_task = (current_task + 1) % task_count;

    if (prev == current_task) {
        return;
    }

    uint32_t *old_esp_ptr;
    uint32_t  new_esp;

    if (prev == -1) {
        static uint32_t dummy;
        old_esp_ptr = &dummy;
    } else {
        task_list[prev]->state = PROC_READY;
        old_esp_ptr = &task_list[prev]->esp;
    }

    task_list[current_task]->state = PROC_RUNNING;
    new_esp = task_list[current_task]->esp;

    switch_context(old_esp_ptr, new_esp);
}
