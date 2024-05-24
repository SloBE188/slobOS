#include "slobos.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"



int main(int argc, char** argv)
{

    printf("My age is %i\n", 19);
    printf("SLOBOS AMOGUS BLANK.ELF PROGRAMM");

    //check if malloc works now
    char *ptr = malloc(20);
    strcpy(ptr, "does it work?");
    print(ptr);
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