[BITS 32]

global _start
extern c_start
extern slobos_exit

section .asm


_start:
    call c_start   ;call th C function from start.c so i can use process arguments becuase i pass the process arguments from c_start to the main function of the loaded process (z.B. blank.elf)
    call slobos_exit ;terminate the process
    ret
