#include "pmm.h"

typedef struct {
    uint32_t base_low;
    uint32_t base_high;
    uint32_t len_low;
    uint32_t len_high;
    uint32_t type;
    uint32_t acpi_attr;
} __attribute__((packed)) e820_entry_t;

#define E820_COUNT_ADDR   0x8000
#define E820_ENTRIES_ADDR 0x8004
#define E820_TYPE_USABLE  1

#define MAX_FRAMES 8192
static uint8_t  bitmap[MAX_FRAMES / 8];

static uint32_t total_frames = 0;
static uint32_t used_frames_count = 0;
static uint32_t highest_frame = 0;

static inline void bitmap_set(uint32_t frame) {
    bitmap[frame / 8] |= (1 << (frame % 8));
}
static inline void bitmap_clear(uint32_t frame) {
    bitmap[frame / 8] &= ~(1 << (frame % 8));
}
static inline int bitmap_test(uint32_t frame) {
    return bitmap[frame / 8] & (1 << (frame % 8));
}

void pmm_init(void) {
    for (uint32_t i = 0; i < MAX_FRAMES / 8; i++) {
        bitmap[i] = 0xFF;
    }
    total_frames = 0;
    used_frames_count = 0;
    highest_frame = 0;

    uint16_t entry_count = *(volatile uint16_t *)E820_COUNT_ADDR;
    e820_entry_t *entries = (e820_entry_t *)E820_ENTRIES_ADDR;

    for (uint16_t i = 0; i < entry_count; i++) {
        e820_entry_t *e = &entries[i];
        if (e->type != E820_TYPE_USABLE) continue;
        if (e->base_high != 0 || e->len_high != 0) continue;

        uint32_t base = e->base_low;
        uint32_t len  = e->len_low;

        if (base < 0x100000) {
            if (base + len <= 0x100000) continue;
            len -= (0x100000 - base);
            base = 0x100000;
        }

        uint32_t start_frame = base / FRAME_SIZE;
        uint32_t frame_count = len / FRAME_SIZE;

        for (uint32_t f = start_frame; f < start_frame + frame_count && f < MAX_FRAMES; f++) {
            bitmap_clear(f);
            total_frames++;
            if (f > highest_frame) highest_frame = f;
        }
    }

    used_frames_count = 0;
}

uint32_t pmm_alloc_frame(void) {
    for (uint32_t f = 0; f <= highest_frame; f++) {
        if (!bitmap_test(f)) {
            bitmap_set(f);
            used_frames_count++;
            return f * FRAME_SIZE;
        }
    }
    return 0;
}

void pmm_free_frame(uint32_t paddr) {
    uint32_t f = paddr / FRAME_SIZE;
    if (f > highest_frame) return;
    if (bitmap_test(f)) {
        bitmap_clear(f);
        if (used_frames_count > 0) used_frames_count--;
    }
}

uint32_t pmm_total_frames(void) { return total_frames; }
uint32_t pmm_used_frames(void)  { return used_frames_count; }
uint32_t pmm_free_frames(void)  { return total_frames - used_frames_count; }