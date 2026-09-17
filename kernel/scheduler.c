/* =============================================================================
 * SENG21213-OS :: Round-Robin Scheduler
 * File   : kernel/scheduler.c
 * Lecture: L09 §3-4
 *
 * Stage 1 milestone (in progress): this file currently just proves the PIT
 * tick is firing and being acknowledged (EOI). PCB/ready-queue/context-switch
 * logic is layered in next.
 * ============================================================================*/
#include "scheduler.h"
#include "pic.h"
#include "vga.h"

static volatile uint32_t tick_count = 0;

void irq0_handler(void) {
    tick_count++;

    /* Visible proof of life: print a dot every 100 ticks (~1 second at 100Hz) */
    if (tick_count % 100 == 0) {
        vga_puts_color(".", VGA_LIGHT_MAGENTA, VGA_BLACK);
    }

    pic_send_eoi(0);
}

void scheduler_init(void) {
    tick_count = 0;
}
