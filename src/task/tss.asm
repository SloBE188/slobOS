section .asm

global tss_load

tss_load:
    push ebp        ; setting up the stack frame
    mov ebp, esp    ; setting up the stack frame
    mov ax, [ebp+8] ; TSS Segment
    ltr ax
    pop ebp         ; clean up the stack frame
    ret             ; return from function(label)