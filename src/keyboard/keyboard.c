#include "keyboard.h"
#include "status.h"
#include "kernel.h"
#include "task/process.h"
#include "task/task.h"
#include "ps2.h"


 //head & tail for the linked list (favourite data structure). linked list is responsible for holding the keyboard drivers
static struct keyboard *keyboard_list_head = 0;
static struct keyboard *keyboard_list_tail = 0;

void keyboard_init()
{

    keyboard_insert(ps2_init());

}


/*With this function i can insert keyboard drivers into my linked list.
if its the first keyboard driver in tke linked list it sets the head and the tail to the provided keyboard driver.
If not it expands the linked list with "->next" and sets the tail to the curr keyboard driver.
After that it initializes the function.*/
int keyboard_insert(struct keyboard *keyboard)
{

    int res = 0;
    if (keyboard->init == 0)
    {
        res = -EINVARG;
        goto out;
    }
    if (keyboard_list_tail)
    {
        keyboard_list_tail->next = keyboard;
        keyboard_list_tail = keyboard;
    }else
    {
        keyboard_list_head = keyboard;
        keyboard_list_tail = keyboard;
    }

    res = keyboard->init();
     
out:
    return res;    
}


/*gibt mir den index im buffer von einem process zurück, wenn ich auf den tail zugreifen will. Der index ist ein bestimmter speicherpunkt im buffer*/
static int keyboard_get_tail_index(struct process *process)
{
    return process->keyboard.tail % sizeof(process->keyboard.buffer);
}


//Setzt den Tail eins runter(-1), berechnet den index des tails und löscht diesen (character wird gelöscht)
void keyboard_backspace(struct process *process)
{
    process->keyboard.tail -= 1;
    int real_index = keyboard_get_tail_index(process);
    process->keyboard.buffer[real_index] = 0x00;
}





/*Setzt den character im paramenter char c(kann irgend eine taste sein auf dem keyboard) an das ende des arrays (tail) also den buffer und 
increments the tail afterwards so the byte does not get overwritten when this function gets used the next timeand sets the tail to its right spot. */
void keyboard_push(char c)
{
    struct process *process = process_current;
    if (!process)
    {
        return;
    }

    int real_index = keyboard_get_tail_index(process);
    process->keyboard.buffer[real_index] = c;
    process->keyboard.tail++;
    
}


/*Diese Funktion popt(löscht) einen key von vorne vom keyboard buffer(head) vom aktuellen process.*/
char keyboard_pop()
{

    if (!process_current)   //cloud be false
    {
        return 0;
    }

    struct process *process = process_current;  //could be false
    //compute the real index in the buffer
    int real_index = process->keyboard.head % sizeof(process->keyboard.buffer);
    char c = process->keyboard.buffer[real_index];
    if (c == 0x00)
    {
        //Nothing to pop becuase in this location at the buffer is nothing stored hihi
        return 0;
    }

    //replace the index in the buffer array with ZEROOO
    process->keyboard.buffer[real_index] = 0;
    process->keyboard.head++;
    return c;
    
}
