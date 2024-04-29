[BITS 32]


global print:function    ;the ":function" marks it being a function for the elf loader (symbol)
global getkey:function


; void print(const char *message)
print:
    push ebp
    mov ebp, esp
    push dword[ebp+8]   ;+8 because the "push ebp" was 4 bytes and the return address (message) from the person who called this function is also 4 bytes
    mov eax, 1 ; command print syscall
    int 0x80    ;interrupt 80 is syscall
    add esp, 4
    pop ebp
    ret


;int getkey();
;eax stores the return value(it can do it as long as the return value isnt bigger than 4 bytes which is the case here)
getkey:
    push ebp
    mov ebp, esp
    mov eax, 2  ;Command getkey
    int 0x80
    pop ebp
    ret




;HOW C COMPILERS CREAT A STACK FRAME:
;push ebp
;
;pop ebp
;ret