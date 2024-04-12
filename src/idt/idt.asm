section .asm

;das c funktionen von idt.c verwendet werden können
extern int21h_handler
extern no_interrupt_handler
extern isr80h_handler
extern interrupt_handler

;macht die labels von aussen erreichbar (C)
global int21h
global idt_load
global no_interrupt
global enable_interrupts
global disable_interrupts
global isr80h_wrapper
global interrupt_pointer_table

;enables interrupts
enable_interrupts:
    sti
    ret


;disables interrupts
disable_interrupts:
    cli
    ret



;lädt die IDT mit dem "lidt" befehl (load interrupt descriptor table). 
;Die IDT ist eine Datenstruktur im OS, welche die Adressen zu den verschiedenen ISR's(Interrupt Service Routinen) enthält.
; dieses label ([ebp+8]) erwartet ein pointer auf die IDT (struct idtr_desc*)
idt_load:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8]
    lidt [ebx]

    pop ebp    
    ret


;Ist die ISR für den interrupt 21h. Ich starte mit dem implementieren von den interrupts bei int 20 (0x20(dez 32)) weil die PIC dort startet,
;die int 0-31 sind pre defined von der CPU also kann ich nur interrupts ab 0x20 implementieren. IRQ0 -IRQ7 verfügbar, IRQ0 (0x20) = System timer, IRQ1 (0x21) = keyboard controller
; Wenn der Interrupts 21h ausgeführt wird, deaktiviert dieses label die interrupts, sichert den aktuellen Prozessorzustand,
;ruft die effektive behandlungsroutine (handler) auf (ist IMMER in C geschrieben(int21h_handler)) welcher dann die aktionen effektiv ausführt, wovür dieser interrupt ist,
;stellt dann den Prozessorzustand wieder her, aktiviert die interrupts wieder
;und kehrt dann aus der interrupt-behandlung zurück (iret)
int21h:
    pushad  ;sichert den aktuellen Prozessorzustand
    call int21h_handler ;ruft interrupt handler (in c geschrieben in idt.c) auf
    popad   ;stellt den alten Prozessorzustand wieder her
    iret    ;kehrt aus der interrupt behandlung zurück

;Ist eine ISR für den interrupt "no_interrupt"
no_interrupt:
    pushad
    call no_interrupt_handler
    popad
    iret




%macro interrupt 1
    global int%1          ; Macht die Funktion int%1 global verfügbar
    int%1:                ; Definiert den Einstiegspunkt für den Interrupt
        ; INTERRUPT FRAME START
        pushad            ; Sichert alle allgemeinen Register (eax, ebx, ecx, edx, esi, edi, ebp, esp)
        push esp          ; Sichert den Stack-Pointer auf dem Stack
        push dword %1     ; Legt die Interrupt-Nummer auf den Stack
        call interrupt_handler ; Ruft die zentrale Interrupt-Handling-Funktion auf (interrupt_handler is a function in C(idt.c))
        add esp, 8        ; Bereinigt den Stack um die beiden zuvor gepushten Werte (esp, Interrupt-Nummer)
        popad             ; Stellt die zuvor gesicherten Register wieder her
        iret              ; Beendet die ISR und stellt den vorherigen Prozesszustand wieder her
%endmacro



;this simply loops 512 times and calls the macro functions which ends up creating the int1, int2, int3 etc. asm interrupt handlers
assign i 0
%rep 512
    interrupt i
%assign i i+1
%endrep
    




;this asm label is the entry point for the interrupt ox80 and is responsible for transferring control to our C code where the request (int command)
;will be processed and once complete this asm label will return to the calling user process.
isr80h_wrapper:
    ;INTERRUPT FRAME START
    ;ALREADY PUSHED BY THE PROCESSOR UPON:
    ;uint32_t ip
    ;uint32_t cs
    ;uint32_t flags
    ;uint32_t sp
    ;uint32_t ss

    ;pushes the general purpose registers (in c the structure interrupt_frame) to the stack (the general purpose registers dont get pushed to the stack automaticly)
    pushad

    ;INTERRUPT FRAME END

    ;push the stack pointer so it points to the interrupt frame (now the isr80h_handler has full acces to the general purpose registers and segment registers that were pushed on the stack).
    ;it uses the struct inerrupt_frame to access all those individual registers
    push esp

    ;EAX hold the command so push it to the stack for the isr80h_handler so it the handler knows which command it has to execute (command in the EAX reg)
    ; inside here (tmp_res) is stored the return result from the isr80h_handler so the result from the command as a example print command (int command 1)
    ; then the result gets moved back to the eax registrer

    push eax
    call isr80h_handler
    mov dword[tmp_res], eax
    add esp, 8


    ;RETURN FROM INTERRUPT
    ;restore general purpose registers
    popad


    mov eax, [tmp_res]

    ;return from the interrupt, restoring the original state of the flags,cs and ip from the stack
    iretd

    section .data
    ;inside here is stored the return result from isr80h_handler
    tmp_res: dd 0


%macro interrupt_array_entry 1
    dd int%1             ; Definiert einen Eintrag in der Tabelle, der die Adresse der Routine int%1 enthält
%endmacro

interrupt_pointer_table:
%assign i 0
%rep 512
    interrupt_array_entry i ; Füllt die Tabelle(array in C) mit den Adressen der Interrupt-Handler
%assign i i+1
%endrep
