
#include "process.h"
#include "task/task.h"
#include "task/process.h"
#include "string/string.h"
#include "status.h"
#include "config.h"
#include "kernel.h"

// Diese Funktion lädt einen Prozess aus der Shell und wechselt zu diesem Prozess.
void* isr80h_command6_process_load_from_shell(struct interrupt_frame* frame)
{
    // Holt den Dateinamen vom Stack des aktuellen Tasks.
    void* filename_user_ptr = task_get_stack_item(task_current(), 0);
    
    // Puffer für den Dateinamen definieren.
    char filename[SLOBOS_MAX_PATH];
    
    // Kopiert den Dateinamen vom User-Space in den Kernel-Space.
    int res = copy_string_from_task(task_current(), filename_user_ptr, filename, sizeof(filename));
    
    // Falls die Kopie fehlschlägt, springt zu 'out' und beendet die Funktion.
    if (res < 0)
    {
        goto out;
    }

    // Definiert den Pfad und fügt den Dateinamen zum Pfad "0:/" hinzu.
    char path[SLOBOS_MAX_PATH];
    strcpy(path, "0:/");
    strcpy(path+3, filename);

    // Zeiger auf den zu ladenden Prozess.
    struct process* process = 0;
    
    // Lädt den Prozess und wechselt zu diesem. Falls der Ladevorgang fehlschlägt, springt zu 'out'.
    res = process_load_switch(path, &process);
    if (res < 0)
    {
        goto out;
    }

    // Wechselt zum Task des geladenen Prozesses (switches page directory)
    task_switch(process->task);
    
    // Stellt die Register und Segment selectors des neuen Tasks her und gibt die Kontrolle an diesen ab.
    task_return(&process->task->registers);

out:
    // Rückgabewert der Funktion. Hier wird immer 0 zurückgegeben.
    return 0;
}

void *isr80h_command7_invoke_system_command(struct interrupt_frame *frame)
{
    struct command_argument* arguments = task_virtual_address_to_physical(task_current(), task_get_stack_item(task_current(), 0));  //returns the physical addres which the kernel can access
    if (!arguments || strlen(arguments[0].argument) == 0)
    {
        return ERROR(-EINVARG);
    }
    //blank.elf arg arg1 arg2 arg3
    struct command_argument* root_command_argument = &arguments[0]; //the root_command_argument contains the program to load, as a example blank.elf
    const char* program_name = root_command_argument->argument;

    char path[SLOBOS_MAX_PATH];
    strcpy(path, "0:/");                                //kopiert "0:/" in den char path
    strncpy(path+3, program_name, sizeof(path));        //kopiert den programmnamen zum char "0:/" dazu, deshalb "path+3", also dann z.B. "0:/blank.elf"

    struct process* process = 0;
    int res = process_load_switch(path, &process);
    if (res < 0)
    {
        return ERROR(res);
    }

    res = process_inject_arguments(process, root_command_argument);
    if (res < 0)
    {
        return ERROR(res);
    }

    task_switch(process->task);
    task_return(&process->task->registers);

    return 0;
}


//this syscall gets the process arguments
void *isr80h_command8_get_program_arguments(struct interrupt_frame *frame)
{

    struct process *process = task_current()->process;
    struct process_arguments *arguments = task_virtual_address_to_physical(task_current(), task_get_stack_item(task_current(), 0));     //returns the physical addres which the kernel can access

    process_get_arguments(process, &arguments->argc, &arguments->argv);
    return 0;

}