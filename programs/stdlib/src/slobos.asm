[BITS 32]


section .asm

global print:function    ;the ":function" marks it being a function for the elf loader (symbol)
global slobos_getkey:function
global slobos_malloc:function
global slobos_free:function
global slobos_putchar:function
global slobos_process_load_from_shell:function
global slobos_process_get_arguments:function
global slobos_system_command:function
global slobos_exit:function


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
slobos_getkey:
    push ebp
    mov ebp, esp
    mov eax, 2  ;Command getkey
    int 0x80
    pop ebp
    ret


;int putchar(int c);
slobos_putchar:
    push ebp
    mov ebp, esp
    mov eax, 3  ;Command 3 putchar
    push dword[ebp+8] ;variable c
    int 0x80
    add esp, 4
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


;void slobos_process_load_from_shell(const char *filename);
slobos_process_load_from_shell:
    push ebp                    ;save base pointer on the stack
    mov ebp, esp                ;set stack pointer to base pointer
    mov eax, 6                  ;syscall 6 (isr80h_command6_process_load_from_shell-> starts a process)
    push dword[ebp+8]           ;push "filename" argument to the stack
    int 0x80                    ;invokes syscall 0x80 (used for syscalls)
    add esp, 4                  ;cleans the stack up 
    pop ebp                     ;restore the base pointer
    ret                         ;returns



; int slobos_system_command(struct command_argument *arguments)
slobos_system_command:
    push ebp
    mov ebp,esp
    mov eax, 7          ; Command 7 process_system_command ( runs a system command based on the arguments)
    push dword[ebp+8]   ;variable "arguments"
    int 0x80
    add esp, 4
    pop ebp
    ret

;void slobos_process_get_arguments(struct process_arguments *arguments)
slobos_process_get_arguments:
    push ebp
    mov ebp, esp
    mov eax, 8  ; Syscall 8 -> gets the process arguments
    push dword[ebp+8]   ;variable arguments (struct process_arguments)
    int 0x80
    add esp, 4
    pop ebp
    ret

slobos_exit:
    push ebp
    mov ebp, esp
    mov eax, 9  ; Syscall 9, terminate the current process
    int 0x80
    pop ebp
    ret






;HOW C COMPILERS CREAT A STACK FRAME:
;push ebp
;
;pop ebp
;ret