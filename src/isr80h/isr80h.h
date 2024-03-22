#ifndef ISR80_H
#define ISR80_H


/*This enum containt a name representation of every command in the system int the C language.*/
enum SystemCommands
{
    SYSTEM_COMMAND0_SUM,        //This kernel command equals 0. So when i set eax to 0 in a user program, this command gets executed.
    SYSTEM_COMMAND1_PRINT,      //eax 1
};

void isr80h_register_commands();

#endif