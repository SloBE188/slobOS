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

// Pointer zum aktuell laufenden Prozess
struct process *current_process = 0;

// Array, das alle Prozesse speichert. Jede Position repräsentiert einen möglichen Prozess-Slot. Das Array hat 12 slots (SLOBOS_MAX_PROCESSES)
static struct process *processes[SLOBOS_MAX_PROCESSES] = {};

// Initialisiert einen Prozess durch NULL des Structs mit memset des Prozesses
static void process_init(struct process *process)
{
    memset(process, 0, sizeof(struct process));
}


// Gibt den aktuellen Prozess zurück, welcher zuletzt auf der CPU executed wurde.
// Da meine implementation für single core CPU's ist, kann nur ein process aktiv auf der cpu laufen, die anderen prozesse sind dann im "paused" or "waiting" state.
//FUnktion ist vor allem wichtig wen ein interrupt invoked wurde, damit ich wen der interrupt returned weis von welchem process der interrupt war.
struct process *process_current()
{
    return current_process;
}

// Holt einen Prozess anhand seiner ID (param)
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

// Lädt ein Binary programm von einem Dateisystem (fat16) in den memory.
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

    // Reserviert Speicher für den process/programm.
    void *program_data_ptr = kzalloc(stat.filesize);        //->kernel sieht direkt diese adresse von kzalloc zurückgegeben.

    if (!program_data_ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    // Liest das Programm in den zuvor reservierten Speicher (program_data_ptr).
    if (fread(program_data_ptr, stat.filesize, 1, fd) != 1)
    {
        res = -EIO;
        goto out;
    }
    
    // hier wird der gemacht, dass die variable "ptr"(physical pointer to the beginning of the process in memory) auf 
    // den startpunkt der zuvor geladenen data vom process zeigt. 
    process->ptr = program_data_ptr;
    process->size = stat.filesize;

out:
    // Schließt die Datei.
    fclose(fd);
    return res;
}

// Wrapper function for process_load_binary
static int process_load_data(const char *filename, struct process *process)
{
    int res = 0;
    res = process_load_binary(filename, process);
    return res;
}

/*Diese Funktion lädt das mit process_load_binary geladene binary file in den virtual memory. Bei mir heisst das das diese Funktion
alle user processes in den virtual memory (starting from 0x400000(#define SLOBOS_PROGRAM_VIRTUAL_ADDRESS 0x400000 )) geladen werdem.
Das bedeutet, dass jeder Process dann seinen eigenen Adressraum hat und nur seinen eigenen Adressraum(4gb) sieht.*/
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

// Wrapper function for process_map_binary
int process_map_memory(struct process *process)
{
    int res = 0;
    res = process_map_binary(process);
    return res;
}

// Lädt und initialisiert einen Prozess für einen mit param "process_slot" gegebenen Slot also im array processes was ganz oben definiert ist.
// Mit dem einfügen eines processes in den gegebenen slot wird auch möglich gemacht, das man zum beispiel mit einem key auf dem keyboard
// z.B. "F2" einen process in den process slot beim array "processes" in den slot 2 laden könnte
// Die funktion allocated auch memory für den prozess.
// EIgentlich bringt diese Funktion alle zuvor erstellten funktionen zusammen.
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
/*this function loops threw the processes array and searches for a free slot. if it finds it. it returns its index.
if not, it shows a error(EISTKN)*/
int process_get_free_slot()
{
    //Loop threw each slot in the "processes" array
    for (int i = 0; i < SLOBOS_MAX_PROCESSES; i++)
    {
        // if i find a empty (null because i memset it by the process_init) slot, return its index
        if (processes[i] == 0)
        {
            return i;
        }
        
    }
    
    //if it reaches this point here in the function this means that all slots are taken and i have to return a error code to signify that no slot is available.
    return -EISTKN;
}

/*this function loads a process from a file with his filename HEHE NICE*/
int process_load(const char* filename, struct process** process)
{
    int res = 0;

    //find the next free slot for a new process
    int process_slot = process_get_free_slot();

    //if no free slot is found, return error
    if (process_slot < 0)
    {
        res = -EISTKN;
        goto out;
    }

    //Load the process into the free slot
    res = process_load_for_slot(filename, process, process_slot);
    
}
