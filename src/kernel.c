#include "kernel.h"
#include <stdint.h>
#include <stddef.h>

uint16_t* video_mem = 0;
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

void kernel_main()
{
    terminal_initialize();
    print("Vamos\nvon Nils");
}