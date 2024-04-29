#include "heap.h"
#include "task/task.h"
#include "task/process.h"
#include <stddef.h>

void* isr80h_command4_malloc(struct interrupt_frame* frame)
{

    size_t size = (int)task_get_stack_item(task_current(), 0)
    return process_malloc(task_current() ->process, size);      //i use "task_current()->process" because thats the one that gets executed here and now, process_current would give me the process im looking at back which would be false

}