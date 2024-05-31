#ifndef SLOBOS_H
#define SLOBOS_H

#include <stddef.h>
#include <stdbool.h>


//linked list Struktur für process arguments. Jede Struktur hat ein Argument und einen Pointer auf das nächste Argument.
struct command_argument
{
    char argument[512];
    struct command_argument *next;  //Pointer zum nächsten Argument in der linked list
};

void print(const char *message);
int slobos_getkey();
void *slobos_malloc(size_t size);
void slobos_free(void *ptr);
int slobos_putchar(int c);


//shell things
void slobos_terminal_readline(char *out, int max, bool output_while_typing);
int slobos_getkeybock();
void slobos_process_load_from_shell(const char *filename);
struct command_argument *slobos_parse_command(const char *command, int max);



#endif