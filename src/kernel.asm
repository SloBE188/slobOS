;Der Kernel läuft nun in einem 32bit mode (Protected Mode), wir können das BIOS nicht mehr nutzen.
[BITS 32]


global _start
global problem
global kernel_registers

extern kernel_main

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

    ;Remap the master PIC
    mov al, 00010001b
    out 0x20, al    ;Tell master PIC

    mov al, 0x20    ;Interrupt 0x20(dez 32) is where master ISR needs to start start because interruprs 0-31 are pre defined for the CPU!!
    out 0x21, al

    mov al, 000000001b
    out 0x21, al
    ;End remap of the master PIC
    ;I maybe need to implement the slave PIC later for more interrupts


    call kernel_main
    jmp $


; this label restores the processors segment registers to kernel land GDT selector offsets without invoking this label the
; processor would continue operating under user process selectors as defined in the GDT (privileges = security problem
kernel_registers:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    ret


times 512- ($ - $$) db 0    ;Extends the File to 512 Bytes (1 Sector)