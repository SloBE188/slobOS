#include "task.h"
#include "kernel.h"
#include "status.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "process.h"

// Globale Variable, die auf den aktuell laufenden Task zeigt.
struct task* current_task = 0;

// Anker für die linked list von Tasks.
struct task* task_tail = 0; //Endpunkt (Schwanz) der Liste (zeigt auf den letzten Task)
struct task* task_head = 0; //Anfangspunkt(Kopf) der Liste (zeigt auf den ersten Task)

// Prototyp für die Initialisierungsfunktion eines Tasks.
int task_init(struct task* task, struct process* process);

// Gibt den aktuell laufenden Task zurück.
struct task* task_current()
{
    return current_task;
}

// Erstellt einen neuen Task und initialisiert ihn.
struct task* task_new(struct process* process)
{
    int res = 0;
    struct task* task = kzalloc(sizeof(struct task));
    // Wenn die Speicherzuweisung fehlschlägt, gebe ENOMEM zurück.
    if (!task)
    {
        res = -ENOMEM;
        goto out;
    }

    // Initialisiere den Task.
    res = task_init(task, process);
    // Bei Fehlschlag, beende.
    if (res != SLOBOS_ALL_OK)
    {
        goto out;
    }

    // Wenn die linked list leer ist, setze Kopf und Schwanz auf den neuen Task.
    if (task_head == 0)
    {
        task_head = task;
        task_tail = task;
        goto out;
    }

    // Füge den neuen Task am Ende der linked list ein.
    task_tail->next = task;
    task->prev = task_tail;
    task_tail = task;

out:    
    // Wenn ein Fehler auftritt, gib den Task-Speicher frei und liefere den Fehler zurück.
    if (ISERR(res))
    {
        task_free(task);
        return ERROR(res);
    }

    return task;
}

// Holt den nächsten Task in der linked list.
struct task* task_get_next()
{
    // Wenn kein nächster Task, starte von vorne.
    if (!current_task->next)
    {
        return task_head;
    }

    return current_task->next;
}

// Entfernt einen Task aus der linked list.
static void task_list_remove(struct task* task)
{
    // Adjustiere die vorherigen und nächsten Zeiger.
    if (task->prev)
    {
        task->prev->next = task->next;
    }

    // Prüfe, ob der Task am Anfang oder Ende ist, und adjustiere entsprechend.
    if (task == task_head)
    {
        task_head = task->next;
    }

    if (task == task_tail)
    {
        task_tail = task->prev;
    }

    // Wenn der zu entfernende Task der aktuelle ist, wechsle zum nächsten.
    if (task == current_task)
    {
        current_task = task_get_next();
    }
}

// Gibt die Ressourcen eines Tasks frei.
int task_free(struct task* task)
{
    // Gibt die Seitentabelle frei.
    paging_free_4gb(task->page_directory);
    // Entfernt den Task aus der Liste.
    task_list_remove(task);

    // Gibt den Speicher des Tasks frei.
    kfree(task);
    return 0;
}

// Initialisiert einen Task.
int task_init(struct task* task, struct process* process)
{
    // Setze den Speicherbereich des Tasks auf 0.
    memset(task, 0, sizeof(struct task));
    // Ordne den gesamten 4GB-Adressraum auf sich selbst zu.
    task->page_directory = paging_new_4gb(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    // Bei Fehlschlag der Seitentabellen-Erstellung, gebe EIO zurück.
    if (!task->page_directory)
    {
        return -EIO;
    }

    // Initialisiere Register (IP, SS, und ESP) für den Task.
    task->registers.ip = SLOBOS_PROGRAM_VIRTUAL_ADDRESS;
    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.esp = SLOBOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START;
    task->process = process;

    return 0;
}
