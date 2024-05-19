
#include "process.h"
#include "task/task.h"
#include "task/process.h"
#include "string/string.h"
#include "status.h"
#include "config.h"


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
