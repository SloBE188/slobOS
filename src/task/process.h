#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "task.h"
#include "config.h"

#define PROCESS_FILETYPE_ELF 0
#define PROCESS_FILETYPE_BINARY 1

typedef unsigned char PROCESS_FILETYPE;

//process allocation struct for processes so i know the size of the allocation, before i only knew the location.
struct process_allocation
{
    void *ptr;
    size_t size;
};

//linked list Struktur für process argumente. Jede Struktur hat ein Argument und einen Pointer auf das nächste Argument.
struct command_argument
{
    char argument[512];
    struct command_argument *next;  //Pointer zum nächsten Argument in der linked list
};

//structure which represents arguments for a process
struct process_arguments
{
    int argc;               //enthält anzahl an argumenten
    char** argv;            //array welches alle argumente enthält
};


//ein process besteht aus einem oder mehreren tasks. ein process ist ein running program auf dem system.
struct process
{

    //ID of the Process
    uint16_t id;

    //The filename of the executable running as this process
    char filename[SLOBOS_MAX_PATH];

    //The main task associated with this process
    struct task *task;

    //The memory allocations made by this process
    //void *allocations[SLOBOS_MAX_PROGRAM_ALLOCATIONS];  //This is a array holding the memory allocations belonging to this process.
    struct process_allocation allocations[SLOBOS_MAX_PROGRAM_ALLOCATIONS];

    //The file can either be binary or elf (as defined above PROCESS_FILETYPE_ELF, PROCESS_FILETYPE_BINARY)
    PROCESS_FILETYPE filetype;
    union
    {
        //The physical pointer to the beginning of the process in memory
        void *ptr;
        struct elf_file *elf_file;
    };
    

    //The physical pointer to the stack memory
    void *stack;

    //The size (in bytes) of the data pointed to by "ptr"
    uint32_t size;

    //keyboard buffer. Every Process has its own Keyboard buffer where it stores when a key gets pressed
    struct keyboard_buffer
    {
        char buffer[SLOBOS_KEYBOARD_BUFFER_SIZE];   //buffer
        int tail;                                   //used for push operations(when a key is pressed a character will be pushed to the end of the buffer dargestellt durch den berechneten index in der tail variable(keyboard_get_tail_index))
        int head;                                   //used for popping from the front of the buffer which would be done when i wanna read what was pushed to the buffer
    }keyboard;
    

    //arguments of the process
    struct process_arguments arguments;
    

};


/*
*filename: representing the name of the file from which nthe process will be loaded
**process: a pointer to a pointer to a struct process, where the loaded processs details will be stored
process_slot: the index of the process in the processes array*/
int process_load_for_slot(const char *filename, struct process **process, int process_slot);


int process_load(const char* filename, struct process** process);

struct process *process_current();
struct process *process_get(int process_id);

//switching processes
int process_switch(struct process *process);
int process_load_switch(const char *filename, struct process **process);


//memory allocation/free for processes in userlans
void *process_malloc(struct process *process, size_t size);
void process_free(struct process *process, void* ptr);

void process_get_arguments(struct process *process, int* argc, char** argv);
int process_inject_arguments(struct process *process, struct command_argument *root_argument);

//terminate process
int process_terminate(struct process* process);

#endif