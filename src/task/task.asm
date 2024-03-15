[BITS 32]
section .asm

global restore_general_purpose_registers
global task_return
global user_registers

;void task_return(struct registers* regs);
;this label is responsible for forcing the os from kernel land into user land where the CPU will begin executing the user process code.
;it simulates the behavior of returning from an interrupt to seamlessly transition the CPU from kernel mode to user mode.
;Die Funktion task_return ist verantwortlich für den Übergang vom Kernel-Modus in den User-Modus.
task_return:

    ; Sichert den aktuellen Stack Pointer im Base Pointer für lokalen Zugriff im folgenden Code.
    mov ebp, esp


    ; Zugriff auf die Struktur, die die CPU-Register und Zustände enthält, indem der Pointer darauf in EBX geladen wird.
    ; BP+4 ist der Ort, wo der Pointer auf die Registerstruktur übergeben wird.
    mov ebx, [ebp+4]

    ; Pushen des User-Modus Stack-Segment-Selectors in den Stack.
    ; Dieser Wert wird von der Registerstruktur geladen (offset 44).
    push dword [ebx+44]

    ; Pushen des User-Modus Stack Pointer in den Stack.
    ; Dieser Wert wird von der Registerstruktur geladen (offset 40).
    push dword [ebx+40]


    ; Speichern der aktuellen Flags in EAX, Anpassen um das Interrupt-Flag zu setzen,
    ; und dann die modifizierten Flags auf den Stack pushen.
    pushf
    pop eax
    or eax, 0x200
    push eax

    ; Pushen des Code-Segment-Selectors für den User-Modus auf den Stack.
    push dword [ebx+32]

    ; Pushen des Instruction Pointers (IP), um die Ausführung fortzusetzen.
    push dword [ebx+28]

    ; Setup der Segment-Register für den User-Modus.
    ; Der DS, ES, FS, und GS Register werden mit dem User-Modus Datensegment-Selektor geladen.
    mov ax, [ebx+44]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Ruft die Funktion auf, die die allgemeinen Register aus der Struktur wiederherstellt.
    ; [ebx+4] ist der Pointer auf die Registerwerte.
    push dword [ebp+4]
    call restore_general_purpose_registers
    add esp, 4  ; Bereinigt den Stack nach dem Funktionsaufruf.



    ; Führt den Übergang in den User-Modus aus: Die iretd Anweisung setzt CS, EIP, EFLAGS,
    ; sowie Stack-Pointer (ESP) und Stack-Segment (SS) aus den Werten, die auf dem Stack liegen.
    ; Nach iretd wird der Code im User-Modus bei der Adresse fortgesetzt, die in EIP gespeichert ist.
    iretd
    

;this label restores the CPU's general purpose registers. this is essential for context switching, as it allows to save
;and restore the state of a task.
; void restore_general_purpose_registers(struct registers* regs);
restore_general_purpose_registers:
    push ebp                    ;pusb base pointer on the stack
    mov ebp, esp                ;set ebp to esp
    mov ebx, [ebp+8]            ;retrieve the address of the struct registers argument passed to the function from the stack and store it in ebx
    mov edi, [ebx]              ;restore the general purpose registers from the values stored in the struct
    mov esi, [ebx+4]            ;restore the general purpose registers from the values stored in the struct
    mov ebp, [ebx+8]            ;restore the general purpose registers from the values stored in the struct
    mov edx, [ebx+16]           ;restore the general purpose registers from the values stored in the struct
    mov ecx, [ebx+20]           ;restore the general purpose registers from the values stored in the struct
    mov eax, [ebx+24]           ;restore the general purpose registers from the values stored in the struct
    mov ebx, [ebx+12]           ;restore the general purpose registers from the values stored in the struct
    pop ebp
    ret

;this label is responsible for updating the CPU's segment registers to user-space selectors defined in the GDT.
;the GDT serves as a central repository that describes various memory segments accessible to the processor.
;Each selector in the GDT corresponds to a specific segment with its own set of permissions and attributes.
;In this context, the 0x23 selector is designed to describe a user-space data segment. It is desinctly seperate from the selectors
;used for kernel space segments. When i load this selector into the segment registers (DS,ES,FS,GS), i tell the CPU to treat
; subsequent memory operations as a user-space operation with his permissions and restrictions. This clear seperation from
;user space and kernel space is for system security and stability so user space tasks cannot accidentally interfere with kernel space ressources.
; void user_registers()
user_registers:
    mov ax, 0x23    ;value 0x23 because this corresponds to the selector for the user data segment in the gdt
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret