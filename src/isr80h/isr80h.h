#ifndef ISR80_H
#define ISR80_H


/*This enum containt a name representation of every command in the system int the C language.*/
enum SystemCommands
{
    SYSTEM_COMMAND0_SUM,        //This kernel command equals 0. So when i set eax to 0 in a user program, this command gets executed.
    SYSTEM_COMMAND1_PRINT,      //eax 1
    SYSTEM_COMMAND2_GETKEY,      //eax 2
    SYSTEM_COMMAND3_PUTCHAR,     //eax 3
    SYSTEM_COMMAND4_MALLOC,      //eax 4
    SYSTEM_COMMAND5_FREE,            //eax 5
    SYSTEM_COMMAND6_PROCESS_LOAD_FROM_SHELL,     //eax 6
    SYSTEM_COMMAND7_INVOKE_SYSTEM_COMMAND,                  //eax 7
    SYSTEM_COMMAND8_GET_PROGRAM_ARGUMENTS                   //eax 8
};

#endif