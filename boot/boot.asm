; =============================================================================
; SENG21213-OS :: Bootloader (Stage 0 base + Stage 3 E820 memory detection)
; File   : boot/boot.asm
; Purpose: MBR bootloader. Loads the kernel, detects the physical memory map
;          via BIOS INT 0x15/E820 (must happen in Real Mode, before PM),
;          then switches to 32-bit Protected Mode and jumps to the kernel.
; Lecture: L11 §1 — Physical memory detection precedes frame allocation
; =============================================================================

[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor  ax, ax
    mov  ds, ax
    mov  es, ax
    mov  ss, ax
    mov  sp, 0x7C00
    sti

    mov  [boot_drive], dl

    mov  si, msg_banner
    call print_rm
    mov  si, msg_load
    call print_rm

; ---------------------------------------------------------------------------
; Load kernel: read sectors 2..65 from disk into memory at 0x1000:0x0000
; ---------------------------------------------------------------------------
load_kernel:
    mov  bx, 0x1000
    mov  es, bx
    xor  bx, bx

    mov  ah, 0x02
    mov  al, 64
    mov  ch, 0
    mov  cl, 2
    mov  dh, 0
    mov  dl, [boot_drive]
    int  0x13
    jc   disk_error

    mov  si, msg_ok
    call print_rm

    ; Reset ES back to 0 before the E820 call below (kernel load left it at 0x1000)
    xor  ax, ax
    mov  es, ax

; ---------------------------------------------------------------------------
; Detect physical memory map via BIOS INT 0x15, EAX=0xE820
; Stores: [0x8000] = entry count (word)
;         [0x8004 ...] = entries, 24 bytes each:
;           +0  base address   (8 bytes)
;           +8  length         (8 bytes)
;           +16 type           (4 bytes: 1=usable, 2=reserved, 3=ACPI reclaim,
;                                4=ACPI NVS, 5=bad)
;           +20 ACPI ext attrs (4 bytes, ignored by our PMM)
; Lecture: L11 §1
; ---------------------------------------------------------------------------
detect_memory:
    mov  di, 0x8004         ; entries start here (ES already 0 from above)
    xor  ebx, ebx           ; continuation value, 0 = start over
    xor  bp, bp             ; entry counter

.e820_loop:
    mov  eax, 0xE820
    mov  ecx, 24
    mov  edx, 0x534D4150    ; 'SMAP' magic
    int  0x15
    jc   .e820_done         ; carry set = unsupported or finished with error

    cmp  eax, 0x534D4150    ; BIOS should return 'SMAP' in EAX too
    jne  .e820_done

    jcxz .e820_skip         ; zero-length entry, skip without counting

    inc  bp
    add  di, 24

.e820_skip:
    test ebx, ebx           ; EBX = 0 means this was the last entry
    jnz  .e820_loop

.e820_done:
    mov  [0x8000], bp       ; store final entry count

    mov  si, msg_mem_ok
    call print_rm

; ---------------------------------------------------------------------------
; Enter Protected Mode
; ---------------------------------------------------------------------------
enter_pm:
    cli
    lgdt [gdt_descriptor]

    mov  eax, cr0
    or   eax, 0x1
    mov  cr0, eax

    jmp  CODE_SEG:init_pm32

[BITS 32]
init_pm32:
    mov  ax, DATA_SEG
    mov  ds, ax
    mov  ss, ax
    mov  es, ax
    mov  fs, ax
    mov  gs, ax

    mov  ebp, 0x90000
    mov  esp, ebp

    call 0x10000

    hlt

[BITS 16]
disk_error:
    mov  si, msg_err
    call print_rm
    mov  si, msg_halt
    call print_rm
    jmp  $

print_rm:
    lodsb
    or   al, al
    jz   .done
    mov  ah, 0x0E
    xor  bh, bh
    int  0x10
    jmp  print_rm
.done:
    ret

boot_drive  db 0

msg_banner  db 13, 10, 'SENG21213-OS Stage 3', 13, 10, 0
msg_load    db '  [BOOT] Loading kernel...', 13, 10, 0
msg_ok      db '  [BOOT] Kernel loaded OK ', 13, 10, 0
msg_mem_ok  db '  [BOOT] Memory map detected', 13, 10, 0
msg_err     db '  [BOOT] DISK ERROR!       ', 13, 10, 0
msg_halt    db '  System halted.           ', 13, 10, 0

gdt_start:
gdt_null:
    dd 0x00000000
    dd 0x00000000

gdt_code:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

times 510 - ($ - $$) db 0
dw 0xAA55
