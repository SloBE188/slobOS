 #include "ps2.h"
 #include "keyboard.h"
 #include "io/io.h"
 #include <stdint.h>
 #include <stddef.h>                    

 int ps2_keyboard_init();


//This is the Scan Code Set 1
//It is a array that maps a scancode to an ASCII character
 static uint8_t keyboard_scan_set1[] = {
    0x00, 0x1B, '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '-', '=',
    0x08, '\t', 'Q', 'W', 'E', 'R', 'T',
    'Y', 'U', 'I', 'O', 'P', '[', ']',
    0x0d, 0x00, 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ';', '\'', '`', 
    0x00, '\\', 'Z', 'X', 'C', 'V', 'B',
    'N', 'M', ',', '.', '/', 0x00, '*',
    0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, '7', '8', '9', '-', '4', '5',
    '6', '+', '1', '2', '3', '0', '.'
};

//Actual structure from this keyboard driver (PS2) which will be in the linked list in the virtual keyboard layer
struct keyboard ps2_keyboard = {
    .name = {"PS2"},
    .init = ps2_keyboard_init
};

//Initizalizes the ps2 keyboard driver
int ps2_keyboard_init()
{
    outb(PS2_PORT, PS2_COMMAND_ENABLE_FIRST_PORT);
    return 0;
}


/*this function takes the parameter scancode which will be converted to a ascii character in this function.
Example: Scancode 0x02 will get converted to character "1"*/
uint8_t ps2_keyboard_scancode_to_char(uint8_t scancode)
{
    size_t size_of_keyboard_set1= sizeof(keyboard_scan_set1) / sizeof(uint8_t);
    if (scancode > size_of_keyboard_set1)
    {
        return 0;
    }

    char c = keyboard_scan_set1[scancode];
    return c;
    
}


void ps2_keyboard_handle_interrupt()
{

}

//Returns a pointer to the keyboard driver
struct keyboard *ps2_init()
{
    return &ps2_keyboard;
}