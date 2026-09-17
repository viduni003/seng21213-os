; =============================================================================
; SENG21213-OS :: IRQ Stub
; File   : boot/irq_stubs.asm
; Purpose: Low-level entry point for IRQ0 (PIT timer). Saves CPU state,
;          calls the C handler, restores state, returns via IRET.
; Lecture: L09 §1 — Interrupt Service Routines
; =============================================================================

[BITS 32]
[EXTERN irq0_handler]   ; C function in kernel/scheduler.c (written next)
[GLOBAL irq0_stub]

irq0_stub:
    pusha                   ; save all general-purpose registers
    call irq0_handler       ; call our C handler
    popa                    ; restore all general-purpose registers
    iret                    ; return from interrupt (pops CS, EIP, EFLAGS)
