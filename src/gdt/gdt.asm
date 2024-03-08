section .asm
global gdt_load


; mit diesem Label wird die GDT in den Prozessor geladen.
gdt_load:
    mov eax, [esp+4]
    mov [gdt_descriptor + 2], eax
    mov ax, [esp+8]
    mov [gdt_descriptor], ax
    lgdt [gdt_descriptor]
    ret


; this gdt descriptor is used by th lgdt instruction to load the gdt in the CPU.
section .data
gdt_descriptor:
    dw 0x00 ; Size (2 bytes)
    dd 0x00 ; GDT Start Address (4 bytes)