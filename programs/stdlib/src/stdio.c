#include "stdio.h"
#include "slobos.h"


int putchar(int c)
{
   slobos_putchar((char*)c);
   return 0;
}