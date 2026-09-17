[BITS 32]
[GLOBAL task_start_trampoline]

task_start_trampoline:
    sti
    jmp ebx
