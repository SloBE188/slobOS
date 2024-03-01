[BITS 32]

section .asm

global paging_load_directory
global enable_paging

paging_load_directory:
    push ebp            ;base pointer value gets saved
    mov ebp, esp        ;base pointer is set to the current stack pointer
    mov eax, [ebp+8]    ;the address of the page directory (ebp+8) gets loaded into the eax register
    mov cr3, eax        ;the content of eax(page directory address) gets stored in the control register (CR3). This action loads the page directorty into the processor, enabling the translation of cirtual to physical addresses
    pop ebp             ;previous base pointer value is restored from the stack
    ret

enable_paging:
    push ebp            ;same
    mov ebp, esp        ;same
    mov eax, cr0        ;value of control register cr0 gets stored in eax
    or eax, 0x80000000  ;a bitwise or operation between the value in eax and the hexadezimal value 0x80000000. this operation sets the 31st bit of CR0, enabling the paging mechanismus
    mov cr0, eax        ;the modified value of eax(with the paging bit set) is stored back into the control register cr0;
    pop ebp             ;same
    ret