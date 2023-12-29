#include "idt.h"
#include "config.h"
#include "memory/memory.h"

extern void idt_load (struct idtr_desc* ptr);

struct idt_desc idt_descriptors[CENTOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

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

    //Divide by zero interrupt (idt_zero) gets maped
    idt_set(0, idt_zero);

    //Load the interrupt descriptor table
    idt_load(&idtr_descriptor);

}
