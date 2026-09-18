/* =============================================================================
 * SENG21213-OS :: Producer-Consumer Demo
 * File   : kernel/producer_consumer.c
 * Lecture: L10 §2-3 — deliverable: producer-consumer demo runs clean,
 *          race condition demo shows the bug when sync is removed
 * ============================================================================*/
#include "vga.h"
#include "mutex.h"
#include "semaphore.h"

#define BUFFER_SIZE 5

static int buffer[BUFFER_SIZE];
static int in_index  = 0;
static int out_index = 0;

static mutex_t     buffer_mutex;
static semaphore_t empty_slots;  /* counts free slots (starts full = BUFFER_SIZE) */
static semaphore_t full_slots;   /* counts filled slots (starts empty = 0) */

static volatile uint32_t produced_count = 0;
static volatile uint32_t consumed_count = 0;

static void spin(volatile uint32_t n) {
    while (n--) { __asm__ __volatile__("nop"); }
}

void pc_init(void) {
    mutex_init(&buffer_mutex);
    sem_init(&empty_slots, BUFFER_SIZE);
    sem_init(&full_slots, 0);
    in_index = 0;
    out_index = 0;
    produced_count = 0;
    consumed_count = 0;
}

/* Producer: waits for a free slot, locks the buffer, inserts an item */
void producer_thread(void) {
    int item = 0;
    while (1) {
        sem_wait(&empty_slots);     /* block until a slot is free */
        mutex_lock(&buffer_mutex);  /* exclusive access to buffer */

        buffer[in_index] = item;
        in_index = (in_index + 1) % BUFFER_SIZE;
        produced_count++;
        item++;

        vga_puts_color("P", VGA_LIGHT_CYAN, VGA_BLACK);

        mutex_unlock(&buffer_mutex);
        sem_signal(&full_slots);    /* signal a new item is available */

        spin(30000000);
    }
}

/* Consumer: waits for an available item, locks the buffer, removes it */
void consumer_thread(void) {
    while (1) {
        sem_wait(&full_slots);      /* block until an item exists */
        mutex_lock(&buffer_mutex);

        int item = buffer[out_index];
        (void)item;
        out_index = (out_index + 1) % BUFFER_SIZE;
        consumed_count++;

        vga_puts_color("C", VGA_LIGHT_RED, VGA_BLACK);

        mutex_unlock(&buffer_mutex);
        sem_signal(&empty_slots);   /* signal a slot is now free */

        spin(40000000);
    }
}

uint32_t pc_get_produced(void) { return produced_count; }
uint32_t pc_get_consumed(void) { return consumed_count; }

/* =============================================================================
 * RACE CONDITION DEMO (no synchronization)
 * Same producer/consumer logic, but WITHOUT mutex or semaphores. Both
 * threads freely read/write shared_counter with no protection — timer
 * preemption mid-increment causes lost updates.
 * ============================================================================*/
static volatile uint32_t shared_counter = 0;

void race_incrementer_a(void) {
    while (1) {
        /* NOT ATOMIC: read, modify, write can be interrupted between
           any of these steps by the other thread doing the same thing */
        uint32_t temp = shared_counter;
        temp = temp + 1;
        shared_counter = temp;
    }
}

void race_incrementer_b(void) {
    while (1) {
        uint32_t temp = shared_counter;
        temp = temp + 1;
        shared_counter = temp;
    }
}

uint32_t race_get_counter(void) { return shared_counter; }
void race_reset_counter(void) { shared_counter = 0; }
