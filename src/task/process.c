#include "process.h"
#include "config.h"
#include "status.h"
#include "task/task.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "string/string.h"
#include "fs/file.h"
#include "kernel.h"

//The current process that is running
struct process *current_process = 0;

static struct process *processes[SLOBOS_MAX_PROCESSES] = {};

static void process_init(struct process *proess)
{
    memset(process, 0, sizeof(struct process));
}

struct process()
{
    return current_process;
}

struct process *process_get(int process_id)
{
    if (process_id < 0 || process_id >= SLOBOS_MAX_PROCESSES)
    {
        return NULL;
    }

    return processes[process_id];

}

static int process_load_binary(const char* filename, struct process *process)
{
    int res = 0;
    int fd = fopen(filename, "r");
    if (!fd)
    {
        res = -EIO;
        goto out;
    }

    struct file_stat stat;
    res = fstat(fd, &stat);

    if(res != SLOBOS_ALL_OK)
    {
        goto out;
    }

    void *program_data_ptr = kzalloc(stat.filesize);

    if (!program_data_ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    if (fread(program_data_ptr, stat.filesize, 1, fd) != 1)
    {
        res = -EIO;
        goto out;
    }
    
    process->ptr = program_data_ptr;
    process->size = stat.filesize;


out:
    fclose(fd);
    return res;

}

static int process_load_data(const char *filename, struct process *process)
{
    int res = 0;
    res = process_load_binary(filename, process);
    return res;
}

int process_map_binary(struct process *process)
{
    int res = 0;
    paging_map_to(process->task->page_directory->directory_entry, (void*) SLOBOS_PROGRAM_VIRTUAL_ADDRESS, process->ptr, paging_align_address(process->ptr + process->size), PAGING_IS_PRESENT|PAGING_ACCESS_FROM_ALL|PAGING_IS_WRITEABLE);
    return res;
}
    