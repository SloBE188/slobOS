#include "kernel.h"
#include <stdint.h>
#include <stddef.h>
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "disk/disk.h"
#include "string/string.h"
#include "fs/pparser.h"
#include "disk/streamer.h"
#include "fs/file.h"
#include "memory/memory.h"
#include "gdt/gdt.h"
#include "config.h"
#include "task/tss.h"

uint16_t *video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;



uint16_t terminal_make_char(char c, char colour)
{

    return (colour << 8) | c;   //fügt die 16 Bits zusammen (c & colour) und verschiebt sie
}

void terminal_putchar(int x, int y, char c, char colour)
{
    video_mem[(y * VGA_WIDTH)+ x] = terminal_make_char(c, colour);
}

//unterschied von putchar function zu writechar function: writechar keeps track of the coloums and rows
void terminal_writechar(char c, char colour)
{
    if (c == '\n')
    {
        terminal_row+= 1;
        terminal_col = 0;
        return;
    }
    terminal_putchar(terminal_col, terminal_row, c, colour);
    terminal_col += 1;
    if(terminal_col >= VGA_WIDTH)
    {
        terminal_col = 0;
        terminal_row += 1;
    }
}

//clear the screen from the BIOS stuff
void terminal_initialize()
{
    video_mem = (uint16_t*)(0xB8000);
    uint16_t terminal_row = 0;
    uint16_t terminal_col = 0;
    for (int y = 0; y < VGA_HEIGHT; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            terminal_putchar(x,y, ' ', 0);
        }
        
    }
    
}


//print routine
void print(const char* str)
{
    size_t len = strlen(str);
    for (int i = 0; i < len; i++)
    {
        terminal_writechar(str[i], 15);
    }
    
}

static struct paging_4gb_chunk* kernel_chunk = 0;

void panic(const char* msg)
{
    print(msg);
    while (1){}
    
}

//GDT
struct tss tss;
struct gdt gdt_real[SLOBOS_TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[SLOBOS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00},                // NULL Segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0x9a},           // Kernel code segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0x92},            // Kernel data segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf8},              // User code segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf2},             // User data segment
    {.base = (uint32_t)&tss, .limit=sizeof(tss), .type = 0xE9}      // TSS Segment
};
void kernel_main()
{
    terminal_initialize();
    print("Vamos\nvon Nils");

    memset(gdt_real, 0x00, sizeof(gdt_real));
    gdt_structured_to_gdt(gdt_real, gdt_structured, SLOBOS_TOTAL_GDT_SEGMENTS);

    // Load the gdt
    gdt_load(gdt_real, sizeof(gdt_real));


    
    //initialize the heap
    kheap_init();


    //Initialize filesystem
    fs_init();

    //Search and initialize the disks
    disk_search_and_init();

    //initialize the IDT  
    idt_init();

    // Setup the TSS
    memset(&tss, 0x00, sizeof(tss));
    tss.esp0 = 0x600000;
    tss.ss0 = KERNEL_DATA_SELECTOR;

    // Load the TSS
    tss_load(0x28);




    //Setting up paging, mapping the entire 4GB of memory linearly to the physical adresses.
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);


    //Switch to kernel paging chunk so that the processor follows the kernel paging directory
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));

    /*PAGING TESTING
    char* ptr = kzalloc(4096);
    paging_set(paging_4gb_chunk_get_directory(kernel_chunk), (void*)0x1000, (uint32_t)ptr | PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITEABLE);
*/
    
    //Enable paging
    enable_paging();

    /*TESTING DISK
    char buf[512];
    disk_read_sector(0, 1, buf);
    Da ich inzwischen den diskdriver angepasst habe (alles asgelagert)
    kann man auch so entwas von der disk lesen:
    disk_read_block(disk_get(0), 20, 4, buf) etc.
    */

    /*PAGING TESTING
    char* ptr2 = (char*) 0x1000;
    ptr2[0] = 'A';
    ptr2[1] = 'B';
    print(ptr2);

    print(ptr);
    */

    //Enable Interrupts
    enable_interrupts();



    /*PATH PARSER TESTING
        struct path_root* root_path = pathparser_parse("0:/bin/shell.exe", NULL);

    if(root_path)
    {


    }*/



    int fd = fopen("0:/hello.txt", "r");
    if (fd)
    {
        print("\nWe opened hello.txt\n");
        char buf[14];
        fseek(fd, 2, SEEK_SET);
        fread(buf, 11, 1, fd);
        buf[13] = 0x00;
        print(buf);
    }
    while(1) {}

    


    //TESTING
    //TESTING
    //TESTING
    //problem();
    //Output oxff here
    //outb(0x60, 0xff);


    /*    
    HEAP TESTING
    void* ptr = kmalloc(50);
    void* ptr2 = kmalloc(5000);
    void* ptr3 = kmalloc(5600);
    kfree(ptr);
    void* ptr4 = kmalloc(50);
    if (ptr || ptr2 || ptr3 || ptr4)
    {
    }
    */
    
}