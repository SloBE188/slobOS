#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include "kernel.h"

//representing a single interrupt in our interrupt descriptor table
struct idt_desc
{
    uint16_t offset_1;  //offset bits 0-15
    uint16_t selector;  //a code segment selector in GDT or LDT
    uint8_t zero;       //unused, set to 0
    uint8_t type_attr;  //type and attributes, see below
    uint16_t offset_2;  //offset bits 16-31
}__attribute__((packed));


//describes the start and the end of the interrupt descriptor table
struct idtr_desc
{
    uint16_t limit;     //size of descriptor table -1
    uint32_t base;      //Base address of the start of the idt (location where the table starts)
}__attribute__((packed));

void idt_init();
void enable_interrupts();
void disable_interrupts();

#endif