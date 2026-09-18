#include "process.h"
#include "vga.h"

static pcb_t   process_table[MAX_PROCESSES];
static uint32_t next_pid = 1;

extern void task_start_trampoline(void);

static void k_strcpy(char *dst, const char *src, size_t max) {
    size_t i = 0;
    while (src[i] && i < max - 1) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

void process_init(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].state = PROC_UNUSED;
        process_table[i].pid = 0;
        process_table[i].name[0] = '\0';
    }
    next_pid = 1;
}

pcb_t *create_process(void (*entry)(void), const char *name) {
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROC_UNUSED) { slot = i; break; }
    }
    if (slot == -1) return NULL;

    pcb_t *p = &process_table[slot];
    p->pid = next_pid++;
    p->state = PROC_READY;
    k_strcpy(p->name, name, sizeof(p->name));

    uint32_t *sp = (uint32_t *)(p->stack + PROCESS_STACK_SIZE);

    *(--sp) = (uint32_t)task_start_trampoline;

    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = (uint32_t)entry;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;

    p->esp = (uint32_t)sp;

    return p;
}

int process_kill(uint32_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != PROC_UNUSED && process_table[i].pid == pid) {
            process_table[i].state = PROC_TERMINATED;
            return 0;
        }
    }
    return -1;
}

pcb_t *process_get_by_pid(uint32_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != PROC_UNUSED && process_table[i].pid == pid) {
            return &process_table[i];
        }
    }
    return NULL;
}

static const char *state_to_string(proc_state_t st) {
    switch (st) {
        case PROC_READY:      return "READY";
        case PROC_RUNNING:    return "RUNNING";
        case PROC_BLOCKED:    return "BLOCKED";
        case PROC_TERMINATED: return "TERMINATED";
        default:              return "UNUSED";
    }
}

static void print_spaces(int count) {
    while (count-- > 0) vga_putchar(' ');
}

static void print_padded_uint(uint32_t n, int width) {
    char buf[16];
    int i = 0;
    if (n == 0) {
        buf[i++] = '0';
    } else {
        uint32_t temp = n;
        char rev[16];
        int r = 0;
        while (temp > 0) {
            rev[r++] = (char)('0' + (temp % 10));
            temp /= 10;
        }
        while (r > 0) {
            buf[i++] = rev[--r];
        }
    }
    buf[i] = '\0';
    vga_puts(buf);
    if (width > i) {
        print_spaces(width - i);
    }
}

static void print_padded_str(const char *str, int width) {
    int len = 0;
    while (str[len]) len++;
    vga_puts(str);
    if (width > len) {
        print_spaces(width - len);
    }
}

void process_print_table(void) {
    vga_puts_color("\n  PID   STATE         NAME\n", VGA_YELLOW, VGA_BLACK);
    vga_puts("  ─────────────────────────────────────\n");
    int count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != PROC_UNUSED) {
            vga_puts("  ");
            print_padded_uint(process_table[i].pid, 6);
            print_padded_str(state_to_string(process_table[i].state), 14);
            vga_puts(process_table[i].name);
            vga_putchar('\n');
            count++;
        }
    }
    if (count == 0) {
        vga_puts("  No active processes.\n");
    }
    vga_putchar('\n');
}


pcb_t *create_thread(void (*entry)(void), const char *name) {
    return create_process(entry, name);
}
