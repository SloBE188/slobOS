#include "kernel.h"
#include <stdint.h>
#include <stddef.h>
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"

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

//Calculates the length of a string
size_t strlen(const char* str)
{
    size_t len = 0;
    while (str[len])
    {
        len++;
    }

    return len;
    
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
extern void problem();
static  struct paging_4gb_chunk* kernel_chunk = 0;

void kernel_main()
{
    terminal_initialize();
    print("Vamos\nvon Nils");
    
    //initialize the heap
    kheap_init();

    //initialize the IDT  
    idt_init();


    //Setting up paging, mapping the entire 4GB of memory linearly to the physical adresses.
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);


    //Switch to kernel paging chunk so that the processor follows the kernel paging directory
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));

    char* ptr = kzalloc(4096);
    paging_set(paging_4gb_chunk_get_directory(kernel_chunk), (void*)0x1000, (uint32_t)ptr | PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITEABLE);

    //Enable paging
    enable_paging();

    char* ptr2 = (char*) 0x1000;
    ptr2[0] = 'A';
    ptr2[1] = 'B';
    print(ptr2);

    print(ptr);

    //Enable Interrupts
    enable_interrupts();


    //TESTING
    //TESTING
    //TESTING
    //problem();
    //Output oxff here
    //outb(0x60, 0xff);
    /*    void* ptr = kmalloc(50);
    void* ptr2 = kmalloc(5000);
    void* ptr3 = kmalloc(5600);
    kfree(ptr);
    void* ptr4 = kmalloc(50);
    if (ptr || ptr2 || ptr3 || ptr4)
    {
    }*/
    
}