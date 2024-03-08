#ifndef TASKSWITCHSEGMENT_H
#define TASKSWITCHSEGMENT_H

#include <stdint.h>
struct tss
{
    uint32_t link;  //used for nesting tasks
    uint32_t esp0; // Kernel stack pointer
    uint32_t ss0; // Kernel stack segment
    uint32_t esp1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t sr3;
    uint32_t eip;       //Instruction pointer (Program counter) ist ein register in der CPU, dass die IMMER Adresse der nächsten auszuführenden Anweusung im proigrammspeicher enthält
    uint32_t eflags;    //spezielles register mit flags
    uint32_t eax;       //eneral Purpose Registers (AX,BX,CX,DX) sind eine Gruppe von Registern in einer CPU, die für verschiedene Aufgaben wie das Halten von Zwischenergebnissen, Adressen und anderen Daten während der Programmausführung verwendet werden können.
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;        //Segment selector: Teil der Segmentierung.  Die Segmentierung ermöglicht es, den Speicher in verschiedene Segmente aufzuteilen, wobei jedes Segment eine unterschiedliche Rolle spielen kann (z. B. Code, Daten, Stack). Ein Segment Selector ist ein Teil eines Segment-Deskriptors, der verwendet wird, um auf ein bestimmtes Segment im Speicher zuzugreifen.
    uint32_t cs         //Segment selector: Teil der Segmentierung.  Die Segmentierung ermöglicht es, den Speicher in verschiedene Segmente aufzuteilen, wobei jedes Segment eine unterschiedliche Rolle spielen kann (z. B. Code, Daten, Stack). Ein Segment Selector ist ein Teil eines Segment-Deskriptors, der verwendet wird, um auf ein bestimmtes Segment im Speicher zuzugreifen.
    uint32_t ss;        //Segment selector: Teil der Segmentierung.  Die Segmentierung ermöglicht es, den Speicher in verschiedene Segmente aufzuteilen, wobei jedes Segment eine unterschiedliche Rolle spielen kann (z. B. Code, Daten, Stack). Ein Segment Selector ist ein Teil eines Segment-Deskriptors, der verwendet wird, um auf ein bestimmtes Segment im Speicher zuzugreifen.
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldtr;
    uint32_t iopb;      //I/O permission bitmap.
} __attribute__((packed));

void tss_load(int tss_segment);
#endif