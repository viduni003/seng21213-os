/* =============================================================================
 * SENG21213-OS :: Stage 1 test processes
 * File   : kernel/test_tasks.c
 * Purpose: Two trivial infinite-loop tasks used to visually prove the
 *          round-robin scheduler is actually alternating between processes.
 * Lecture: L09 §4 — deliverable: "two user processes running concurrently"
 * ============================================================================*/
#include "vga.h"

/* Busy-wait so each task's output is visible instead of flashing by instantly */
static void spin(volatile uint32_t n) {
    while (n--) { __asm__ __volatile__("nop"); }
}

void task_a(void) {
    while (1) {
        vga_puts_color("A", VGA_LIGHT_GREEN, VGA_BLACK);
        spin(2000000);
    }
}

void task_b(void) {
    while (1) {
        vga_puts_color("B", VGA_LIGHT_RED, VGA_BLACK);
        spin(2000000);
    }
}
