#ifndef CONFIG_H
#define CONFIG_H


#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0X10

//IDT
#define SLOBOS_TOTAL_INTERRUPTS 512 //200h



//HEAP
//100MB heap size
#define SLOBOS_HEAP_SIZE_BYTES 104857600        //total size of the heap
#define SLOBOS_HEAP_BLOCK_SIZE 4096             //the size of eacht block in the heap (4096 bytes)
#define SLOBOS_HEAP_ADDRESS 0x01000000          //the starting address of the heap in memory
#define SLOBOS_HEAP_TABLE_ADDRESS 0x00007E00    //address where the heap table is in memory (heap table describes every entry(block) in the heap)

//disk
#define SLOBOS_SECTOR_SIZE 512

//filesystem
#define SLOBOS_MAX_FILESYSTEMS 12
#define SLOBOS_MAX_FILE_DESCRIPTORS 512
#define SLOBOS_MAX_PATH 108

//GDT
#define SLOBOS_TOTAL_GDT_SEGMENTS 6


//Tasks
#define SLOBOS_PROGRAM_VIRTUAL_ADDRESS 0x400000                 //defines the starting address where the user programs will be loaded in virtual memory (PAGING)
#define SLOBOS_USER_PROGRAM_STACK_SIZE 1024 * 16                //defines size of the stack for user programs
#define SLOBOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START 0x3FF000     //defines starting addres of the user programs virtual stack
#define SLOBOS_PROGRAM_VIRTUAL_STACK_ADDRESS_END SLOBOS_PROGRAM_VIRTUAL_ADDRESS_START - SLOBOS_USER_PROGRAM_STACK_SIZE  //defines the end address of the user programs virtual stack

//Processes
#define SLOBOS_MAX_PROGRAM_ALLOCATIONS 1024                     //defines a max of memory allocations a user program can make
#define SLOBOS_MAX_PROCESSES 12                                 //defines a max of processes that can run concurrently (gleichzeitig)



#define USER_DATA_SEGMENT 0x23  //data segment for user land
#define USER_CODE_SEGMENT 0x1b  //code segment for user land



#endif
