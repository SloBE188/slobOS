section .asm

;das c funktionen von idt.c verwendet werden können
extern int21h_handler
extern no_interrupt_handler

;macht die labels von aussen erreichbar (C)
global int21h
global idt_load
global no_interrupt
global enable_interrupts
global disable_interrupts

;enables interrupts
enable_interrupts:
    sti
    ret


;disables interrupts
disable_interrupts:
    cli
    ret



;lädt die IDT mit dem "lidt" befehl (load interrupt descriptor table). 
;Die IDT ist eine Datenstruktur im OS, welche die Adressesn zu den verschiedenen ISR's(Interrupt Service Routinen) enthält.
; dieses label ([ebp+8]) erwartet ein pointer auf die IDT (struct idtr_desc*)
idt_load:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8]
    lidt [ebx]

    pop ebp    
    ret


;Ist eine ISR für den interrupt 21h. Wenn der Interrupts 21h ausgeführt wird, deaktiviert dieses label die interrupts, sichert den aktuellen Prozessorzustand,
;ruft die effektive behandlungsroutine (handler) auf (ist IMMER in C geschrieben(int21h_handler)) welcher dann die aktionen effektiv ausführt, wovür dieser interrupt ist,
;stellt dann den Prozessorzustand wieder her, aktiviert die interrupts wieder
;und kehrt dann aus der interrupt-behandlung zurück (iret)
int21h:
    cli     ;deaktiviert interrupts
    pushad  ;sichert den aktuellen Prozessorzustand
    call int21h_handler ;ruft interrupt handler (in c geschrieben in idt.c) auf
    popad   ;stellt den alten Prozessorzustand wieder her
    sti     ;aktiviert interrupts
    iret    ;kehrt aus der interrupt behandlung zurück

;Ist eine ISR für den interrupt "no_interrupt"
no_interrupt:
    cli
    pushad
    call no_interrupt_handler
    popad
    sti
    iret