[BITS 32]

global _start
extern c_start

section .asm


_start:
    call c_start   ;call th C function from start.c
    ret     ;terminate the process
