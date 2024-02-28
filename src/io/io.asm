section .asm

;macht sie von aussen erreichbar (C)
global insb
global insw
global outb
global outw


;label reads 8 bits (1 byte) from a specified I/O port. das gelesene wird dem register dx übergeben und dann in das unterste byte des al registers geladen (Teil des eax registers) 
insb:
    push ebp        ;push ebp to the stack for a ebp "backup".
    mov ebp, esp    ;move esp to ebp. Now i can "play" with esp.

    xor eax, eax
    mov edx, [ebp+8]
    in al, dx

    pop ebp         ;pop ebp to restore ebp from the top of the stack
    ret

;label reads 16 bits (2 bytes) from a specified I/O port. das gelesene wird dem register dx übergeben und dann in das ax register geladen.
insw:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov edx, [ebp+8]
    in ax, dx

    pop ebp
    ret

;label writes 8 bits (1 byte) to a specified I/O port. das value zum schreiben wird in das register eax übergeben und der zielport ins dx register. 
;Das value wird in das unterste byte (al) des eax registers geladen.
outb:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov eax, [ebp+12]
    mov edx, [ebp+8]
    out dx, al

    pop ebp
    ret


;label writes 16 bits (2 bytes) to a specified I/O port. das value zum schreiben wird in das register eax übergeben und der zielport ins dx register. 
;Das value wird in das gesamte ax-Register (niedrigste 16 bits von eax) rein geladen.
outw:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov eax, [ebp+12]
    mov edx, [ebp+8]
    out dx, ax

    pop ebp
    ret