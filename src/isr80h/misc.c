#include "misc.h"
#include "idt/idt.h"
#include "task/task.h"



//Implementation of the "isr80h_commando_sum" command. It is here because it is a miscellaneous command which is for test purposes and
//doesnt fit into any particular category
//The result of the kernel command is ALWAYS stored in the eax register.
void* isr80h_commando_sum(struct interrupt_frame* frame)
{

    int v2 = (int) task_get_stack_item(task_current(), 1);      //extracts the 20 from the tasks stack (pushed from the blank.asm user program). 20 is stored at index 1 because the stack grows downwards
    int v1 = (int) task_get_stack_item(task_current(), 0);      //extracts the 30 from the tasks stack (pushed from the blank.asm user program)
  
    return (void*)(v1 + v2);
}

