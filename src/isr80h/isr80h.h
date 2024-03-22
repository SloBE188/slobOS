#ifndef ISR80_H
#define ISR80_H


/*This enum containt a name representation of every command in the system int the C language.*/
enum SystemCommands
{
    SYSTEM_COMMAND0_SUM,
};

void isr80h_register_commands();

#endif