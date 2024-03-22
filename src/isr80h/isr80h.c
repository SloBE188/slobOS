#include "isr80h.h"
#include "idt/idt.h"
#include "misc.h"


/*registers every command with the "isr80h_register_command" function in the idt.c file*/
void isr80h_register_commands()
{
    isr80h_register_command(SYSTEM_COMMAND0_SUM, isr80h_commando_sum);
}