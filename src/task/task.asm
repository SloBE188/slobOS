[BIOTS 32]
section .asm

global user_registers


;this label is responsible for updating the CPU's segment registers to user the user-space selectors defined in the GDT.
;the GDT serves as a central repository that describes various memory segments accessible to the processor.
;Each selector in the GDT corresponds to a specific segment with its own set of permissions and attributes.
;In this context, the 0x23 selector is designed to describe a user-space data segment. It is desinctly seperate from the selectors
;used for kernel space segments. When i load this selector into the segment registers (DS,ES,FS,GS), i tell the CPU to treat
; subsequent memory operations as a user-space operation with his permissions and restrictions. This clear seperation from
;user space and kernel space is for system security and stability so user space tasks cannot accidentally interfere with kernel space ressources.
user_registers:
    mov ax, 0x23    ;value 0x23 because this corresponds to the selector for the user data segment in the gdt
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret


;this label restores the CPU's general purpose registers. this is essential for context switching, as it allows to save
;and restore the state of a task.
restore_general_purpose_registers:
    push ebp                ;pusb base pointer on the stack
    mov ebp, esp            ;set ebp to esp
    mov ebx, [ebp+8]        ;retrieve the address of the struct registers argument passed to the function from the stack and store it in ebx
    mov edi, [ebx]          ;restore the general purpose registers from the values stored in the struct
    mov esi, [ebx+4]        ;restore the general purpose registers from the values stored in the struct
    mov ebp, [ebx+8]        ;restore the general purpose registers from the values stored in the struct
    mov edx, [ebx+16]       ;restore the general purpose registers from the values stored in the struct
    mov ecx, [ebx+20]       ;restore the general purpose registers from the values stored in the struct
    mov eax, [ebx+24]       ;restore the general purpose registers from the values stored in the struct
    mov ebx, [ebx+12]       ;restore the general purpose registers from the values stored in the struct
    add esp, 4
    ret




;this label is responsible for forcing the os from kernel land into user land where the CPU will begin executing the user process code.
;it simulates the behavior of returning from an interrupt to seamlessly transition the CPU from kernel mode to user mode.

task_return:
    mov ebp, esp
    ;the following lines prepare the stack of iretd instruction
    ;by pushing segment selectors and other state information

    ;access the structure passed to me
    mov ebx, bp+4

    ;push the data/stack selector
    push dword [ebx+44]
    ;push the stack pointer
    push dword [ebx+40]

    ;push the flags
    pushf
    pop eaxor eax, 0x200
    push eax

    ;push the code segment
    push dword [ebx+32]

    ;push the IP(instruction pointer) to execute
    push dword [ebx+28]

    ;setup some segment registers
    mov ax, [ebx+44]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push dword [ebx+4]
    call restore_general_purpose_registers
    add esp, 4

    ;transition into user land!!!!!!!
    iretd