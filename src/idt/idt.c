#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "io/io.h"

struct idt_desc idt_descriptors[SLOBOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;


//import asm functions
extern void idt_load (struct idtr_desc* ptr);
extern void int21h();
extern void no_interrupt();


//This handler wil print "Keyboard pressed" whenever a krey gets pressed
void int21h_handler()
{

    print("Keyboard pressed\n");
    /*Send this to the PIC to acknowledge i have handled the interrupt*/
    outb(0x20,0x20); // communicating with the pic is achieved through I/O Ports (command port=0x20, data port=0x21)
}
/*This no interrupt handler will be used when their is no
associated interrupt routine for an interrupt number*/
void no_interrupt_handler()
{
    /*Send this to the PIC to acknowledge i have handled the interrupt*/
    outb(0x20,0x20); // communicating with the pic is achieved through I/O Ports (command port=0x20, data port=0x21)
}

//Divide by zero interrupt
void idt_zero()
{
    print("Divide by zero error\n");
}

//funktion wird gebraucht, um die idt table mit den interrupts zu füllen. man muss der funktion die interrupts nummer und die adresse der zuständigen ISR mitliefern.
/* in der IDT sind direkt die Adressen der ISR (Interrupt Service Routines) gespeichert. Wenn ein Interrupt auftritt, verwendet der Prozessor die IDT,
 um zu bestimmen, welche ISR aufgerufen werden soll. Diese ISR, die in Assembly geschrieben sind, fungieren als erste Anlaufstelle,
 um den aktuellen Prozessorzustand zu sichern und die Umgebung für einen sicheren Aufruf des eigentlichen Interrupt-Handlers (in C in disem file geschrieben) vorzubereiten.*/
void idt_set(int interrupt_no, void* address)
{
    struct idt_desc* desc = &idt_descriptors[interrupt_no];
    desc->offset_1 = (uint32_t) address & 0x0000ffff;
    desc->selector = KERNEL_CODE_SELECTOR;
    desc->zero = 0x00;
    desc->type_attr = 0xEE;
    desc->offset_2 = (uint32_t) address >> 16;
}

void idt_init()
{
    //memset the idt (das keine random werte drinn sind sondern nur Nullen)
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    // This is for the actual table size from the IDT
    idtr_descriptor.limit = sizeof(idt_descriptors) -1;
    /*This is the base pointer of where the idt is stored. We pass the pointer and cast it to
    uint32_t to stoire it in the descriptor*/
    idtr_descriptor.base = (uint32_t) idt_descriptors;


    //This loop sets all interrupts to point to the no interrupt routine so every interrupt (i have 512) has a ISR assigned to it. When i add a interrupt i have to add them manually
    //like with the int21h (idt_set(0x21, int21h);)
    for (int i = 0; i < SLOBOS_TOTAL_INTERRUPTS; i++)
    {
        idt_set(i, no_interrupt);
    }
    

    //Divide by zero interrupt (idt_zero) gets maped
    idt_set(0, idt_zero);

    /*Here i set the keyboard interrupt 0x21 (IRQ1 in the PIC) to point to the int21h
    handler which is defined in asm. The int21h handler eventually
    calls the int21h_handler C function defined above here.
    int21h is in the src/idt/idt.asm file*/
    idt_set(0x21, int21h);

    //Load the interrupt descriptor table
    idt_load(&idtr_descriptor);

}

