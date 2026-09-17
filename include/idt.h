/* =============================================================================
 * SENG21213-OS :: Interrupt Descriptor Table
 * File   : include/idt.h
 * Lecture: L09 §1 — Interrupt-driven scheduling requires a working IDT
 * ============================================================================*/
#ifndef IDT_H
#define IDT_H

#include "types.h"

/* One IDT entry (8 bytes) — describes one interrupt/exception handler */
typedef struct {
    uint16_t base_low;   /* handler address bits 0-15  */
    uint16_t sel;        /* code segment selector (0x08 from our GDT) */
    uint8_t  always0;    /* reserved, must be 0 */
    uint8_t  flags;      /* type + attributes */
    uint16_t base_high;  /* handler address bits 16-31 */
} __attribute__((packed)) idt_entry_t;

/* Pointer structure loaded by the LIDT instruction */
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

void idt_set_gate(uint8_t num, uint32_t handler, uint16_t sel, uint8_t flags);
void idt_init(void);

#endif /* IDT_H */
