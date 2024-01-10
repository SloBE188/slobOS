#ifndef CONFIG_H
#define CONFIG_H


#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0X10

//IDT
#define CENTOS_TOTAL_INTERRUPTS 512 //200h



//HEAP
//100MB heap size
#define CENTOS_HEAP_SIZE_BYTES 104857600
#define CENTOS_HEAP_BLOCK_SIZE 4096
#define CENTOS_HEAP_ADDRESS 0x01000000
#define CENTOS_HEAP_TABLE_ADDRESS 0x00007E00

//disk
#define CENTOS_SECTOR_SIZE 512

//filesystem
#define CENTOS_MAX_FILESYSTEMS 12
#define CENTOS_MAX_FILE_DESCRIPTORS 512

#endif
