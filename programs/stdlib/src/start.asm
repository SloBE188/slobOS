[BITS 32]

global _start
extern c_start

section .asm


_start:
    call c_start   ;call th C function from start.c so i can use process arguments becuase i pass the process arguments from c_start to the main function of the loaded process (z.B. blank.elf)
    ret     ;terminate the process
