# SENG21213-OS

A small x86 (i686) operating system built from scratch in C99 (freestanding) and NASM assembly, for SENG 21213 — Computer Architecture and Operating Systems, University of Kelaniya.

Boots via a custom 512-byte MBR bootloader into 32-bit Protected Mode, with no underlying OS — every driver, the scheduler, memory manager, and file system are implemented here.

## Build & Run

Requires: `nasm`, `gcc` + `gcc-multilib`, `make`, `qemu-system-i386`, `gdb`, `binutils` (see Student Guide for install instructions per OS).

```bash
make clean && make    # build seng21213-os.img
make run               # build (if needed) and launch in QEMU
```

Click inside the QEMU window for keyboard focus. Close QEMU with Ctrl+C in the terminal.

## Stages Implemented

### Stage 0 — Boot, VGA & Shell (`v0.1-stage0`)
- 512-byte MBR bootloader (NASM), switches to 32-bit Protected Mode via a flat GDT
- VGA text-mode driver (`kernel/vga.c`) writing directly to `0xB8000`
- PS/2 keyboard driver (`kernel/keyboard.c`), polling mode, Scan Code Set 1
- Interactive kernel shell (`ksh>`)

**Try:** `help`, `clear`, `about`, `echo <text>`, `mem`

### Stage 1 — Process Table & Round-Robin Scheduler (`v0.2-stage1`)
- IDT (`kernel/idt.c`) and 8259 PIC remap (`kernel/pic.c`) — IRQ0-7 moved to vectors 0x20-0x27, clear of CPU exception vectors
- PIT (`kernel/pit.c`) programmed to fire IRQ0 at 100 Hz
- `pcb_t` (`include/process.h`) with a fake initial stack frame built by `create_process()`, letting a never-run process resume identically to an interrupted one
- Context switch (`boot/switch.asm`) using a consistent `popa; ret` pattern (not `iret`) for every switch, fresh or resumed — the interrupt stub's own `iret` handles the real return
- Round-robin scheduler (`kernel/scheduler.c`) in the IRQ0 handler
- Shell itself registered as a schedulable task (`create_current_process`), so it keeps receiving CPU time alongside other processes

**Try:** `ps` — lists all processes with PID, state, and name

### Stage 2 — Threads, Mutex & Semaphore (`v0.3-stage2`)
- Kernel threads (`create_thread()`) sharing the kernel's address space, built on Stage 1's PCB machinery
- `mutex_t` (`kernel/mutex.c`) — lock/unlock via interrupt disable (valid on this uniprocessor, interrupt-driven-preemption kernel)
- Counting `semaphore_t` (`kernel/semaphore.c`) — `sem_wait`/`sem_signal`
- Producer-consumer demo (`kernel/producer_consumer.c`): bounded 5-slot buffer, mutex for exclusive access, two semaphores (empty/full slot counts)
- Race condition demo: same increment logic *without* synchronization, showing lost updates from concurrent unprotected read-modify-write

**Try:**
- `ps` — see `shell`, `producer`, `consumer` running concurrently (watch for `P`/`C` output)
- `race` — spawns two unsynchronized threads incrementing a shared counter for ~3 seconds; reports the final count, which is non-deterministic across runs due to lost updates

### Stage 3 — Physical Memory Manager (`v0.4-stage3`)
- `boot/boot.asm` extended with BIOS `INT 0x15, EAX=0xE820` memory detection in Real Mode (must happen before switching to Protected Mode), storing the memory map at `0x8000`
- `kernel/pmm.c` parses the E820 data and builds a bitmap (1 bit per 4KB frame) covering all usable RAM from 1MB upward
- `pmm_alloc_frame()` / `pmm_free_frame()` — first-fit bitmap allocator

**Try:**
- `meminfo` — total/used/free physical frames and KB
- `memtest` — allocates and frees 100 frames in a loop, verifies the free-frame count returns to baseline with zero leaks

### Stage 4 — RAM Disk File System (`v0.5-stage4`)
- 1MB RAM disk (`kernel/fs.c`) — a fixed-size byte array in BSS
- On-disk layout: superblock (block 0), flat directory (block 1), inode table (block 2), block bitmap (block 3), data blocks (block 4+)
- `fs_inode_t`: size + 8 direct block pointers (32KB max file size)
- Flat directory of `(name, inode)` pairs
- Basic file operations: `fs_create`, `fs_unlink`, `fs_write`, `fs_read`, `fs_list`

**Try:**