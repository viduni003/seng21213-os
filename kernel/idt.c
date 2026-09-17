/* =============================================================================
 * SENG21213-OS :: Interrupt Descriptor Table implementation
 * File   : kernel/idt.c
 * Lecture: L09 §1
 * ============================================================================*/
#include "idt.h"
extern void irq0_stub(void);

static idt_entry_t idt[256];
static idt_ptr_t   idt_ptr;

/* Defined in idt_flush.asm — loads IDT via LIDT instruction */
extern void idt_flush(uint32_t);

void idt_set_gate(uint8_t num, uint32_t handler, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = handler & 0xFFFF;
    idt[num].base_high = (handler >> 16) & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags;
}

void idt_init(void) {
    idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idt_ptr.base  = (uint32_t)&idt;

    /* Zero the whole table first — unset entries must be inert */
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    /* Vector 0x20 = IRQ0 (PIT timer), after PIC remap */
    idt_set_gate(0x20, (uint32_t)irq0_stub, 0x08, 0x8E);

    idt_flush((uint32_t)&idt_ptr);
}
