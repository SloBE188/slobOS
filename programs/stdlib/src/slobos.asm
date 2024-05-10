[BITS 32]


section .asm

global print:function    ;the ":function" marks it being a function for the elf loader (symbol)
global getkey:function
global slobos_malloc:function
global slobos_free:function


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


;void* slobos_malloc(size_t size)
slobos_malloc:
    push ebp
    mov ebp, esp
    mov eax, 4  ;command malloc (4)
    push dword[ebp+8]   ;variable "size"
    int 0x80
    add esp, 4      ;restore stack frame
    pop ebp
    ret



; void slobos_free(void *ptr)
slobos_free:
    push ebp
    mov ebp, esp
    mov eax, 5      ;command 5 free in the int 80
    push dword[ebp+8]   ;Variable "ptr"
    int 0x80
    add esp, 4
    pop ebp
    ret




;HOW C COMPILERS CREAT A STACK FRAME:
;push ebp
;
;pop ebp
;ret