#ifndef KEYBOARD_H
#define KEYBOARD_H

struct process;


/*function pointer to the init function of all keyboard drivers in the OS*/
typedef int (*KEYBOARD_INIT_FUNCTION)();

/*Jeder keyboard driver ist eingekapselt in einem keyboard struct HEHEHEHEHA*/
struct keyboard
{
    KEYBOARD_INIT_FUNCTION init;    //init function of a keyboard driver
    char name[20];
    struct keyboard *next;
};

void keyboard_init();
int keyboard_insert(struct keyboard *keyboard);
void keyboard_backspace(struct process *process);
void keyboard_push(char c);
char keyboard_pop();



#endif