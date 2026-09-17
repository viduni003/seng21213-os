; =============================================================================
; SENG21213-OS :: IDT Flush
; File   : boot/idt_flush.asm
; Purpose: Loads the IDT pointer via the LIDT instruction. Called once from
;          idt_init() in kernel/idt.c.
; Lecture: L09 §1
; =============================================================================

[BITS 32]
[GLOBAL idt_flush]

idt_flush:
    mov eax, [esp+4]   ; grab the idt_ptr_t* argument off the stack
    lidt [eax]         ; load the IDT
    ret
