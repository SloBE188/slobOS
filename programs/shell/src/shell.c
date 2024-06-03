#include "shell.h"
#include "stdio.h"
#include "stdlib.h"
#include "slobos.h"


int main (int argc, char **argv)
{
    print("SlobOS V1.0.0\n");
    while (1)
    {
        print(">");
        char buf[1024];
        slobos_terminal_readline(buf, sizeof(buf), true);
        print("\n");
        
        //slobos_process_load_from_shell(buf);
        slobos_system_run(buf);
        print("\n");
    }
    
}