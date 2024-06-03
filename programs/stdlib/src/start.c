#include "slobos.h"

extern int main(int argc, char** argv);


void c_start()
{
    
    struct process_arguments arguments;     //struct with the injected process arguments (arguments get inejected with the "process_inject_arguments" function)
    slobos_process_get_arguments(&arguments);   //get the process arguments with the syscall 8

    int res = main(arguments.argc, arguments.argv); //pass them to the main function from the loaded process (z.B. blank.elf or shell.elf)
    if (res == 0)
    {
        /* code */
    }
    
}