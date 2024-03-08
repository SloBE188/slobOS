#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "task.h"
#include "config.h"


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
    void *allocations[SLOBOS_MAX_PROGRAM_ALLOCATIONS];  //This is a array holding the memory allocations belonging to this process.

    //The physical pointer to the beginning of the process in memory
    void *ptr;

    //The physical pointer to the stack memory
    void *stack;

    //The size (in bytes) of the data pointed to by "ptr"
    uint32_t size;

};





#endif