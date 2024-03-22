[BITS 32]
section .asm

global _start


_start:
    push 20         ;push 20 to the tasks stack
    push 30         ;push 30 to the tasks stack
    mov eax, 0      ;Command 0 (SUM)
    int 0x80
    add esp, 8      ;Restore the stack pointer
    jmp $