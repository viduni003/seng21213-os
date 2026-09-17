[BITS 32]
[GLOBAL switch_context]

switch_context:
    pusha
    mov eax, [esp+36]
    mov edx, [esp+40]
    mov [eax], esp
    mov esp, edx
    popa
    ret
