/* =============================================================================
 * SENG21213-OS :: Stage 1 test processes
 * File   : kernel/test_tasks.c
 * Purpose: Two tasks used to visually prove the round-robin scheduler is
 *          alternating between processes at different rates.
 * Lecture: L09 §4 — deliverable: "two user processes running concurrently"
 * ============================================================================*/
#include "vga.h"
#include "../include/types.h"

/* Busy-wait so each task's output is visible instead of flashing by instantly */
static void spin(volatile uint32_t n) {
    while (n--) { __asm__ __volatile__("nop"); }
}

static void draw_task_status(int row, int col, const char *label, uint32_t count, uint8_t attr) {
    int c = col;
    while (*label) {
        vga_write_cell(row, c++, *label++, attr);
    }
    char num[5];
    num[0] = (char)('0' + ((count / 1000) % 10));
    num[1] = (char)('0' + ((count / 100) % 10));
    num[2] = (char)('0' + ((count / 10) % 10));
    num[3] = (char)('0' + (count % 10));
    num[4] = '\0';
    for (int i = 0; i < 4; i++) {
        vga_write_cell(row, c++, num[i], attr);
    }
    vga_write_cell(row, c++, ']', attr);
}

void task_a(void) {
    uint32_t count = 0;
    uint8_t attr = VGA_ATTR(VGA_LIGHT_GREEN, VGA_BLACK);
    while (1) {
        count++;
        draw_task_status(0, 52, "[A:", count, attr);
        spin(2000000);
    }
}

void task_b(void) {
    uint32_t count = 0;
    uint8_t attr = VGA_ATTR(VGA_LIGHT_RED, VGA_BLACK);
    while (1) {
        count++;
        draw_task_status(0, 65, "[B:", count, attr);
        spin(5000000); /* slower spin so task_b ticks at a different rate */
    }
}
