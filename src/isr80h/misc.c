#include "misc.h"
#include "idt/idt.h"



//Implementation of the "isr80h_commando_sum" command. It is here because it is a miscellaneous command which is for test purposes and
//doesnt fit into any particular category
//The result of the kernel command is ALWAYS stored in the eax register. So in this kernel command i return 69!!
void* isr80h_commando_sum(struct interrupt_frame* frame)
{

    int a = 69;
    return a;
}

