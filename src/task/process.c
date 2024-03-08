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

// Globale Variable, die auf den aktuell laufenden Prozess zeigt.
struct process *current_process = 0;

// Array, das alle Prozesse speichert. Jede Position repräsentiert einen möglichen Prozess-Slot.
static struct process *processes[SLOBOS_MAX_PROCESSES] = {};

// Initialisiert einen Prozess durch Nullsetzen seiner Struktur.
static void process_init(struct process *process)
{
    memset(process, 0, sizeof(struct process));
}

// Gibt den aktuellen Prozess zurück.
struct process *current_process()
{
    return current_process;
}

// Holt einen Prozess anhand seiner ID.
struct process *process_get(int process_id)
{
    // Prüft, ob die angegebene ID gültig ist.
    if (process_id < 0 || process_id >= SLOBOS_MAX_PROCESSES)
    {
        return NULL;
    }

    // Gibt den Prozess an der spezifizierten ID zurück.
    return processes[process_id];
}

// Lädt ein Programm/Binary von einem Dateisystem in den Speicher.
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

    if (res != SLOBOS_ALL_OK)
    {
        goto out;
    }

    // Reserviert Speicher für das Programm.
    void *program_data_ptr = kzalloc(stat.filesize);

    if (!program_data_ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    // Liest das Programm in den zuvor reservierten Speicher.
    if (fread(program_data_ptr, stat.filesize, 1, fd) != 1)
    {
        res = -EIO;
        goto out;
    }
    
    // Speichert den Zeiger und die Größe des Programms im Prozess-Objekt.
    process->ptr = program_data_ptr;
    process->size = stat.filesize;

out:
    // Schließt die Datei.
    fclose(fd);
    return res;
}

// Wrapper-Funktion, die das Laden von Binärdaten eines Programms behandelt.
static int process_load_data(const char *filename, struct process *process)
{
    int res = 0;
    res = process_load_binary(filename, process);
    return res;
}

// Karte das geladene Programm in den virtuellen Adressraum des Prozesses.
int process_map_binary(struct process *process)
{
    int res = 0;
    paging_map_to(process->task->page_directory->directory_entry, 
                  (void*) SLOBOS_PROGRAM_VIRTUAL_ADDRESS, 
                  process->ptr, 
                  paging_align_address(process->ptr + process->size), 
                  PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);
    return res;
}

// Wrapper-Funktion, die das Mapping des Programm-Speichers durchführt.
int process_map_memory(struct process *process)
{
    int res = 0;
    res = process_map_binary(process);
    return res;
}

// Lädt und initialisiert einen Prozess für einen gegebenen Slot.
int process_load_for_slot(const char *filename, struct process **process, int process_slot)
{
    int res = 0;
    struct task* task = 0;
    struct process* _process;
    void* program_stack_ptr = 0;

    // Prüft, ob der Slot bereits belegt ist.
    if (process_get(process_slot) != 0)
    {
        res = -EISTKN;
        goto out;
    }

    // Reserviert Speicher für den Prozess.
    _process = kzalloc(sizeof(struct process));

    if (!_process)
    {
        res = -ENOMEM;
        goto out;
    }

    // Initialisiert den Prozess.
    process_init(_process);
    
    // Lädt die Binärdaten des Programms.
    res = process_load_data(filename, _process);
    if (res < 0)
    {
        goto out;
    }

    // Reserviert Speicher für den Programmstack.
    program_stack_ptr = kzalloc(SLOBOS_USER_PROGRAM_STACK_SIZE);

    if (!program_stack_ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    // Speichert Dateinamen, Stack-Zeiger und ID im Prozess.
    strncpy(_process->filename, filename, sizeof(_process->filename));
    _process->stack = program_stack_ptr;
    _process->id = process_slot;

    // Erstellt eine Task-Struktur für den Prozess.
    task = task_new(_process);
    if (ERROR_I(task) == 0)
    {
        res = ERROR_I(task);
        goto out;
    }

    _process->task = task;

    // Mappt den Speicher des Programms in den Adressraum.
    res = process_map_memory(_process);
    if (res < 0)
    {
        goto out;
    }

    *process = _process;

    // Fügt den Prozess zum Prozess-Array hinzu.
    processes[process_slot] = _process;

out:
    // Wenn ein Fehler aufgetreten ist, gebe die Ressourcen frei.
    if (ISERR(res))
    {
        if (_process && _process->task)
        {
            task_free(_process->task);
        }
        // Hier sollten Sie den Prozessdatenspeicher freigeben, wenn nötig.
    }
    
    return res;
}
