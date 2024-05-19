[BITS 32]

global _start
extern main

section .asm


_start:
    call main   ;call th C function from blank.c or shell.c
    ret     ;terminate the process
