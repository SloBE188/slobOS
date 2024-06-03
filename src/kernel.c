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
#include "task/task.h"
#include "task/process.h"
#include "status.h"
#include "isr80h/isr80h.h"
#include "keyboard/keyboard.h"

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


//backspace
void terminal_backspace()
{
    if (terminal_row == 0 && terminal_col ==0 )
    {
        return;
    }
    
    if (terminal_col == 0)
    {
        terminal_row -= 1;
        terminal_col = VGA_WIDTH;
    }
    
    terminal_col -= 1;
    terminal_writechar(" ", 15);
    terminal_col -= 1;
    
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

    if (c == 0x08)
    {
        terminal_backspace();
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

//error handling mechanismus for unforseen internal system errors where the kernel cant recover from
void panic(const char* msg)
{
    print(msg); //panic msg
    while (1){} //infinite loop to halt operations
    
}


/*This function is designed to swap the currently active page tables from the user process back to the kernels own page tables &
restore the processors segment registers to the kernel one*/
void kernel_page()
{

    //Call the kerne_registers functione
    kernel_registers();

    //invoke the paging_switch function to switch to the kernels page tables
    paging_switch(kernel_chunk);
}

//GDT
struct tss tss;
struct gdt gdt_real[SLOBOS_TOTAL_GDT_SEGMENTS];         //mal recherchieren 
struct gdt_structured gdt_structured[SLOBOS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00},                // NULL Segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0x9a},           // Kernel code segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0x92},            // Kernel data segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf8},              // User code segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf2},             // User data segment
    {.base = (uint32_t)&tss, .limit=sizeof(tss), .type = 0xE9}      // TSS Segment
};

void pic_timer_interrupt(struct interrupt_frame *frame)
{
    print("Timer interrupt works\n");
}


void kernel_main()
{
    terminal_initialize();

    memset(gdt_real, 0x00, sizeof(gdt_real));           //sets the segments of the gdt to 0x00 so no random values are there
    gdt_structured_to_gdt(gdt_real, gdt_structured, SLOBOS_TOTAL_GDT_SEGMENTS); //form the gdt_structured to gdt so the pc understands it

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
    tss.ss0 = KERNEL_DATA_SELECTOR;     // set the kernel stack segment selector to the value i set before (0x10)

    // Load the TSS
    tss_load(0x28);




    //Setting up paging, mapping the entire 4GB of memory linearly to the physical adresses.
    //Damit habe ich nun den Adressraum "kernel_chunk" erstellt. wenn ich verschiedene prozesse habe und jeder prozess ein eigener adressbereich haben muss,
    //damit sie von einander isoliert sind, muss ich einfach für jeden prozess seinen eigenen adresraum erstellen. Zum Beispiel so: prozess1 = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);


    //Switch to kernel paging chunk so that the processor follows the kernel paging directory
    //Hier wechsle ich in das page directory von "kernel_chunk". ich könnte mit der paging switch funktion auch in andere paging directorys von zum beispiel anderen prozessen wechseln
    paging_switch(kernel_chunk);

    /*PAGING TESTING
    char* ptr = kzalloc(4096);
    paging_set(paging_4gb_chunk_get_directory(kernel_chunk), (void*)0x1000, (uint32_t)ptr | PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITEABLE);
*/
    
    //Enable paging
    enable_paging();

    //Register the kernel commands
    isr80h_register_commands();

    //Initialize the keyboard drivers with the virtual keyboard layer
    keyboard_init();


    //TIMER INTERRUPT WITH NEW IDT SYSTEM
    //idt_register_interrupt_callback(0x20, pic_timer_interrupt);

    

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
    //enable_interrupts();



    /*PATH PARSER TESTING
        struct path_root* root_path = pathparser_parse("0:/bin/shell.exe", NULL);

    if(root_path)
    {


    }*/



    /*
    FILESYSTEM TESTING
    int fd = fopen("0:/hello.txt", "r");
    if (fd)
    {
        print("\nWe opened hello.txt\n");
        char buf[14];
        fseek(fd, 2, SEEK_SET);
        fread(buf, 11, 1, fd);
        buf[13] = 0x00;
        print(buf);
    }*/



    struct process* process = 0;
    int res = process_load_switch("0:/shell.elf", &process);
    if (res != SLOBOS_ALL_OK)
    {
        panic("Failed to load shell.elf\n");
    }


    //Create a argument with the value (argv) "Testing"
    struct command_argument argument;
    strcpy(argument.argument, "Testing");       //copy the string "Testing" into the before created "command_argument" structure
    argument.next = 0x00;


    //inject the argument with the value  into the process
    process_inject_arguments(process, &argument);

    //switches to the first task in the linked list and executes it
    task_run_first_ever_task();

    while(1) {}

    /*NO CODE BELOW HERE WILL BE RUN ABTER THE ABOVE FUNCTION CALL IM EXECUTING IN
    USER LAND THE USER PROCESS UNTIL AN INTERRUPT OCCURS*/
    





    


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