#include "misc.h"
#include "idt/idt.h"



//Implementation of the "isr80h_commando_sum" command. It is here because it is a miscellaneous command which is for test purposes and
//doesnt fit into any particular category
void* isr80h_commando_sum(struct interrupt_frame* frame)
{
    return 0;
}

