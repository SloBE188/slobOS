#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include "kernel.h"

//representing a single interrupt in our interrupt descriptor table, jeder eintrag (entry) in der idt ist jeweils 8 bytes lang
struct idt_desc
{
    uint16_t offset_1;  //offset bits 0-15, Offset_1 (2 Bytes): Die ersten 16 Bits der Basisadresse der ISR. Sie geben den niederwertigen Teil der Adresse an, an die gesprungen werden soll, wenn der Interrupt auftritt.
    uint16_t selector;  //a code segment selector in GDT or LDT, Selector (2 Bytes): Das Segment-Selektor-Feld. Dies gibt das Segment an, in dem die ISR zu finden ist, üblicherweise das Code-Segment des Kernels.
    uint8_t zero;       //unused, set to 0, Zero (1 Byte): Ein reserviertes Feld, das immer auf 0 gesetzt wird.
    uint8_t type_attr;  //type and attributes, see below, Type_attr (1 Byte): Dieses Feld definiert verschiedene Eigenschaften des IDT-Eintrags, einschließlich des Gate-Typs (z.B. Task Gate, Interrupt Gate, Trap Gate) und des DPL (Descriptor Privilege Level), das die Privilegien-Ebene angibt, die erforderlich ist, um die ISR zu aktivieren.
    uint16_t offset_2;  //offset bits 16-31, Offset_2 (2 Bytes): Die letzten 16 Bits der Basisadresse der ISR. Sie geben den höherwertigen Teil der Adresse an.
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