#include "slobos.h"





//function loops until a key is pressed, if a key is pressed, it returns th keys integer value
int slobos_getkeyblock()
{
    int val = 0;
    do
    {
        val = slobos_getkey();
    }
    while(val == 0);
    return val;
}




/*
@param out = pointer to the output string (normally a buffer)
@param max = size of the string
@param output_while_typing = boolean if character should be written to the terminal as they are typed 
it loops until the max character count is read or the enter key is pressed. the it checks if output_while_typing is true, if its the case it writes the terminal
Backspaces are handled by checking for the kex 0x08 and if its not at the inputs's start. the output strings current character becomes null
and the loop counter decrements by e (given the upcoming increment by 1 at the loops conclusion)
*/
void slobos_terminal_readline(char* out, int size, bool output_while_typing)
{
    int i = 0;
    for (i = 0; i < size -1; i++)
    {
        char key = slobos_getkeyblock();

        // user klicks enter 
        if (key == 13)
        {
            break;
        }

        if (output_while_typing)
        {
            slobos_putchar(key);
        }

        // Backspace
        if (key == 0x08 && i >= 1)
        {
            out[i-1] = 0x00;
            // -2 because i will +1 when i continue
            i -= 2;
            continue;
        }

        out[i] = key;
    }

    // Add the null terminator
    out[i] = 0x00;
}