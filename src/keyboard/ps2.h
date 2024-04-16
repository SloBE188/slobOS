#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H

#define PS2_PORT 0x64   //port will be used for the communication through outb with function. (I/O)
#define PS2_COMMAND_ENABLE_FIRST_PORT 0xAE  //this value gets passed to the PS2 port for enabling him and thus allowing to receive key presses from the keyboard through IRQ1 aka interrupt 0x21 in the PIC
#define PS2_KEYBOARD_KEY_RELEASED  0x80     //When a key is released, its scan coe is prefixed by ox80
#define ISR_KEYBOARD_INTERRUPT 0x21         //This is the IRQ (in the PIC) for the keyboard
#define KEYBOARD_INPUT_PORT 0x60            //This is the Port Number where the keyboard sends its data (keyboard input). it is essential for reading characters from the keyboards



struct keyboard *ps2_init();



#endif