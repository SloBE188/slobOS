#ifndef HEAP_H
#define HEAP_H

#include "config.h"

//These system headers are built into the cross compiler, thats why i can use it.
#include <stdint.h>
#include <stddef.h>

#define HEAP_BLOCK_TABLE_ENTRY_TAKEN 0X01   //dieses flag zeigt, das der block in use ist
#define HEAP_BLOCK_TABLE_ENTRY_FREE 0x00    //flag zeigt, das der block frei ist un allocated werden kann

//Multi-block allocations (mehr als 4096 bytes)
#define HEAP_BLOCK_HAS_NEXT 0b10000000  //flag signalisiert, das der current block und der nächste nicht frei sind (schon allocated)
#define HEAP_BLOCK_IS_FIRST 0b01000000  //flag zeigt, das der aktuelle blockder erste von mehrere nicht allocateten blöcken ist

typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;


/*Heap Table ist die Datenstruktur, welche den status von jedem einzelnen block in unserem heap speichert (jeder HEAP_BLOCK_TABLE_ENTRY ist jeweils ein block of memory (4096 bytes)). 
Jeder Entry ist 4097 Bytes gross
und der Heap ist 4096 byte aligned, das heisst, wenn ich ein malloc für 5000 bytes mache, werden 2 blöcke (8192) im memory (heap data pool) allocated */
struct heap_table
{
    HEAP_BLOCK_TABLE_ENTRY* entries;
    size_t total;
};

struct heap
{
    struct heap_table* table;

    //Start address of the heap data pool as an example 0x200000
    void* saddr;
    
};

int heap_create (struct heap* heap, void* ptr, void* end, struct heap_table* table);
void* heap_malloc(struct heap* heap, size_t size);
void* heap_free(struct heap* heap, void* ptr);


 #endif