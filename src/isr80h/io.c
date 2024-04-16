#include "io.h"
#include "task/task.h"
#include "kernel.h"
#include "keyboard/keyboard.h"


void* isr80h_command1_print(struct interrupt_frame* frame)
{
    void* user_space_msg_buffer = task_get_stack_item(task_current(), 0);
    char buf[1024];
    copy_string_from_task(task_current(), user_space_msg_buffer, buf, sizeof(buf));

    print(buf);
    return 0;
}


/*this function retrieves a character from the front of the current keyboard buffer and returns it*/
void* isr80h_command2_getkey(struct interrupt_frame* frame)
{
    char c = keyboard_pop();
    return (void*)((int)c);
}

/*this syscall extracts a character from the current tasks stack and writes it to the terminal in white (15)*/
void* isr80h_command3_putchar(struct interrupt_frame* frame)
{
    char c = (char)(int) task_get_stack_item(task_current(), 0);
    terminal_writechar(c, 15);
    return 0;
}