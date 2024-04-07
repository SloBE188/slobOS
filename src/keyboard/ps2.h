#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H

#define PS2_PORT 0x64   //port will be used for the communication through outb with function. (I/O)
#define PS2_COMMAND_ENABLE_FIRST_PORT 0xAE  //this value gets passed to the PS2 port for enabling him and thus allowing to receive key presses from the keyboard through ISR1 aka interrupt 0x21 in the PIC

struct keyboard *ps2_init();



#endif