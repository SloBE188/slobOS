#include "slobos.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"



int main(int argc, char** argv)
{

    //print(argv[0]);
    for (int i = 0; i < argc; i++)
    {
        printf("%s\n", argv[i]);
    }
    
    slobos_exit();


    /*truct process_arguments arguments;
    slobos_process_get_arguments(&arguments);
    printf("%i %s", arguments.argc, arguments.argv[0]);
    printf("My age is %i\n", 19);
    printf("SLOBOS AMOGUS BLANK.ELF PROGRAMM");

    //check if malloc works now
    char *ptr = malloc(20);
    strcpy(ptr, "does it work?");
    print(ptr); 

    char str[] = "hello slobeworld";
    struct command_argument* root_command = slobos_parse_command(str, sizeof(str));
    printf("%s\n", root_command->argument);
    printf("%s\n", root_command->next->argument);*/
    /*
    char words[] = "hello does it work";

    const char* token = strtok(words, " ");
    while(token)
    {
        printf("%s\n", token);
        token = strtok(NULL, " ");
    }
    */
    

    /*
    print(itoa(8763));

    putchar('Z');

    void* ptr = malloc(512);
    free(ptr);

    char buf[1024];
    slobos_terminal_readline(buf, sizeof(buf), true);

    print(buf);
    */
    while(1) 
    {
    }
    return 0;
}