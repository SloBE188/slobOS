;Der Kernel läuft nun in einem 32bit mode (Protected Mode), wir können das BIOS nicht mehr nutzen.
[BITS 32]


global _start


CODE_SEG equ 0x08   ;Offsetwert
DATA_SEG equ 0x10   ;Offsetwert
_start:         
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp

    ;Enable A20 Line
    in al, 0x92
    or al, 2
    out 0x92, al

    jmp $

times 512- ($ - $$) db 0    ;Extends the File to 512 Bytes (1 Sector)