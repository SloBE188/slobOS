#include "slobos.h"
#include "stdlib.h"
#include "stdio.h"

int main(int argc, char **argv)
{
    print("*********WELCOME TO SLOBOS*********\n");
    print(itoa(38));
    putchar('B');
    void *ptr = malloc(512);
    free(ptr);

    while (1)
    {
        if (getkey() != 0)
        {
            print("key pressed\n");
        }
    }

    while (1)
    {
    }

    return 0;
}