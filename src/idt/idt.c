#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "io/io.h"

struct idt_desc idt_descriptors[CENTOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

extern void idt_load (struct idtr_desc* ptr);
extern void int21h();
extern void no_interrupt();


//This handler wil print "Keyboard pressed" whenever a krey gets pressed
void int21h_handler()
{

    print("Keyboard pressed\n");
    /*Send this to the PIC to acknowledge we have handled the interrupt*/
    outb(0x20,0x20);
}
/*This no interrupt handler will bne used when their is no
associated interrupt routine for an interrupt number*/
void no_interrupt_handler()
{
    /*Send this to the PIC to acknowledge we have handled the interrupt*/
    outb(0x20,0x20); 
}

//Divide by zero interrupt
void idt_zero()
{
    print("Divide by zero error\n");
}

//This function is used to map an interrupt to an address
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
    //memset will be implemented soon
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    // This is for the actual table size
    idtr_descriptor.limit = sizeof(idt_descriptors) -1;
    /*This ist the base pointer of wehere our idt is stored. We pass the pointer and cast it to
    uint32_t to stoire it in the descriptor*/
    idtr_descriptor.base = (uint32_t) idt_descriptors;


    //This loop sets all interrupts to point to the no interrupt routine
    for (int i = 0; i < CENTOS_TOTAL_INTERRUPTS; i++)
    {
        idt_set(i, no_interrupt);
    }
    

    //Divide by zero interrupt (idt_zero) gets maped
    idt_set(0, idt_zero);

    /*Here we set out keyboard interrupt 0x21 to point to our int21h
    handler which is defined in asm. The int21h handler eventually
    calls the int21h_handler C function defined above here.
    int21h is in the src/idt/idt.asm file*/
    idt_set(0x21, int21h);

    //Load the interrupt descriptor table
    idt_load(&idtr_descriptor);

}