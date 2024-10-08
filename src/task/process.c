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
#include "loader/formats/elfloader.h"

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

    // Reserviert Speicher für den process/programm. Dieser speicher wird später mit der process map binary funktion in den adressraum geladen und es werden virtuelle adresse zu diesem allocated memory zugewiesen mit der paging_map_to function
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
    process->filetype = PROCESS_FILETYPE_BINARY;        //sets the filetype to binary 
    process->ptr = program_data_ptr;
    process->size = stat.filesize;

out:
    // Schließt die Datei.
    fclose(fd);
    return res;
}

//Lädt ein elf file in den memory (ist wie die process laod binary function einfach für elf files)
int process_load_elf(const char *filename, struct process *process)
{
    int res = 0;
    struct elf_file *elf_file = 0;
    res = elf_load(filename,&elf_file);
    if (res < 0)
    {
        goto out;
    }
    
    process->filetype = PROCESS_FILETYPE_ELF;
    process->elf_file = elf_file;

out:
    return res;
}

// Wrapper function for process_load_binary & process_load_elf
static int process_load_data(const char *filename, struct process *process)
{
    int res = 0;
    res = process_load_elf(filename, process);

    //checks if the file isnt a elf file and the loads the binary
    if (res == -EINFORMAT)
    {
        res = process_load_binary(filename, process);
    }
    return res;
}

/*Diese Funktion lädt das mit process_load_binary geladene binary file in den virtual memory. Bei mir heisst das das diese Funktion
alle user processes in den virtual memory (starting from 0x400000(#define SLOBOS_PROGRAM_VIRTUAL_ADDRESS 0x400000 )) geladen werdem.
Das bedeutet, dass jeder Process dann seinen eigenen Adressraum hat und nur seinen eigenen Adressraum(4gb) sieht.*/
int process_map_binary(struct process *process)
{
    int res = 0;
    paging_map_to(process->task->page_directory, 
                  (void*) SLOBOS_PROGRAM_VIRTUAL_ADDRESS, 
                  process->ptr, 
                  paging_align_address(process->ptr + process->size), 
                  PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);
    return res;
}


/*Diese Funktion lädt das mit process_load_ELF geladene ELF file in den virtual memory. Bei mir heisst das das diese Funktion
alle user processes in den virtual memory (starting from 0x400000(#define SLOBOS_PROGRAM_VIRTUAL_ADDRESS 0x400000 )) geladen werdem.
Das bedeutet, dass jeder Process dann seinen eigenen Adressraum hat und nur seinen eigenen Adressraum(4gb) sieht.*/
int process_map_elf(struct process *process)
{
    int res = 0;

    struct elf_file *elf_file = process->elf_file;
    struct elf_header *header = elf_header(elf_file);
    struct elf32_phdr *phdrs = elf_pheader(header);
    //loops threw all the program headers
    for (int i = 0; i < header->e_phnum; i++)
    {
        // Greife auf den i-ten programmheader zu und speichere die Adresse.
        struct elf32_phdr *phdr = &phdrs[i];

        // Bestimme die physikalische Adresse des Segmentanfangs
        void *phdr_phys_address = elf_phdr_phys_addr(elf_file, phdr);

        // Setze die Standard-Flags für Paging, die angibt, dass die Page 
        // anwesend ist und von allen Zugriffsebenen aus zugegriffen werden kann.
        int flags = PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL;

        // Überprüfe, ob das Segment das Flag PF_W (PF_W Flag) enthält.
        // Wenn ja, füge das PAGING_IS_WRITEABLE hinzu.
        if (phdr->p_flags & PF_W)
        {
            flags |= PAGING_IS_WRITEABLE;       //hinzufügen vom flag PAGING_IS_WRITEABLE
        }

        // Führe die Paging zuodnung durch:
        // - Verwende das page directory des Prozesses.
        // - Die virtuelle Adresse (phdr->p_vaddr) wird zur nächstniedrigeren pagegrenze ausgerichtet.
        // - Die physikalische Startadresse wird ebenfalls zur pagegrenze ausgerichtet.
        // - Die physikalische Endadresse des Segmentes wird auf die nächsthöhere pagegrenze ausgerichtet.
        // - Wende die zuvor definierten Flags an.
        res = paging_map_to(
            process->task->page_directory,
            paging_align_to_lower_page((void*)phdr->p_vaddr),
            paging_align_to_lower_page(phdr_phys_address),
            paging_align_address(phdr_phys_address + phdr->p_memsz),
            flags);

        // Überprüfe das Ergebnis der Seitenzuordnung.
        // Falls ein Fehler auftritt (ISERR(res) liefert true), brich die Schleife ab.
        if (ISERR(res))
        {
            break;
        }
    }
        
    return res;
}

// Wrapper function for process_map_binary & process_map_elf
int process_map_memory(struct process *process)
{
    int res = 0;

    //hier wird ausgewählt, welche funktion für das laden des user processes in den virtual memory (adressraum) genutzt werden muss, es gibt zwei arten welche unterstützt werden, elf und binary
    switch (process->filetype)
    {
    case PROCESS_FILETYPE_ELF:
        res = process_map_elf(process);
    break;

    case PROCESS_FILETYPE_BINARY:
        res = process_map_binary(process);
    break;

    default:
        panic("process_map_memory: Invalid filetype (not binary and not elf)\n");
    }

    if (res < 0)
    {
        goto out;
    }

    //memory map the tasks stack hehe
    paging_map_to(process->task->page_directory, (void*) SLOBOS_PROGRAM_VIRTUAL_STACK_ADDRESS_END, process->stack, paging_align_address(process->stack+SLOBOS_USER_PROGRAM_STACK_SIZE), PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);
    
out:
    return res;
}

// Lädt und initialisiert einen Prozess für einen mit param "process_slot" gegebenen Slot also im array processes was ganz oben definiert ist.
// Mit dem einfügen eines processes in den gegebenen slot wird auch möglich gemacht, das man zum beispiel mit einem key auf dem keyboard
// z.B. "F2" einen nprocess in den process slot beim array "processes" in den slot 2 laden könnte
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

out:
    return res;
    
}


//this function changes the currently loaded process to the new process argument provided(Therefore swithcing the active process)
//so it just switches the current_process variable to the new process which is provided as a parameter
int process_switch(struct process *process)
{
    current_process = process;
    return 0;
}


//function checks for a free place in the allocations array if it finds one, it returns it. else res = -ENOMEM
static int process_find_free_allocation_index(struct process *process)
{
    int res = -ENOMEM;
    for (int i = 0; i < SLOBOS_MAX_PROGRAM_ALLOCATIONS; i++)
    {
        if (process->allocations[i].ptr == 0)
        {
            res = i;
            break;
        }
        
    }
    return res;

}

//Malloc function for proceses (systemcall isr80h_command4_malloc, eax 4);
void *process_malloc(struct process *process, size_t size)
{

    //allocated memory
    void *ptr = kzalloc(size);
    if (!ptr)
    {
        goto out_error;
    }

    //finds a free place in the array
    int index = process_find_free_allocation_index(process);
    if (index < 0)
    {
        goto out_error;
    }
    
    //map the allocated memory to the processes adressraum so i can use it in the process. If i dont do this a page fault occurs
    int res = paging_map_to(process->task->page_directory, ptr, ptr, paging_align_address(ptr+size), PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    //check if the memory map was succesfull
    if (res < 0)
    {
        goto out_error;
    }
    
    //stores the place in the ptr and returns it
    process->allocations[index].ptr = ptr;
    process->allocations[index].size = size;
    return ptr;

out_error:
    if (ptr)
    {
        kfree(ptr);
    }

    return 0;
    
}


//this function is for the process free function (to free memory in processes which i allocated with malloc). it loops threw all the use process allocations
//and checks if the ptr to the allocation(param ptr) is there or not
static bool process_is_process_pointer(struct process *process, void *ptr)
{
    for (int i = 0; i < SLOBOS_MAX_PROGRAM_ALLOCATIONS; i++)
    {
        if (process->allocations[i].ptr == ptr)
        {
            return true;
        }
        
    }

    return false;
    
}


//leaves the "ptr" param from the allocation array in user processes
static void process_unjoin_allocation_array(struct process *process, void *ptr)
{

    //loops threw all the process allocations and sets the "ptr" one to 0x00
    for (int i = 0; i < SLOBOS_MAX_PROGRAM_ALLOCATIONS; i++)
    {
        if (process->allocations[i].ptr == ptr)
        {
            process->allocations[i].ptr = 0x00;
            process->allocations[i].size = 0;
        }
        
    }
    

}

static struct process_allocation *process_get_allocation_by_addr(struct process *process, void *addr)
{

    for (int i = 0; i < SLOBOS_MAX_PROGRAM_ALLOCATIONS; i++)
    {
        if (process->allocations[i].ptr == addr)
        {
            return &process->allocations[i].ptr;
        }
        
    }
    return 0;
    
}

//frees memory allocations from processes.
void process_free(struct process *process, void* ptr)
{
    //checks with the process_is_process_pointer functio if the ptr is in the program allocations array or not (if not i cant free it)
    if (!process_is_process_pointer(process, ptr))
    {
        return;
    }

    //unlink the pages from the process for the given address
    struct process_allocation *allocation = process_get_allocation_by_addr(process, ptr);
    if (!allocation)
    {
        return;
    }

    //here the magic (unlink the pages from the previos allocated memory) happens. I can do this with the paging_map_to function
    //so i simply map it again but this time without the flags (is present, access from all, writeable), this time i set the flags to 0x00 so the process wont be able to access it again.(the kernel can, he can access everything)
    int res = paging_map_to(process->task->page_directory, allocation->ptr, allocation->ptr, paging_align_address(allocation->ptr+allocation->size), 0x00);
    if (res < 0)
    {
        return;
    }
    

    //unjoin the allocation array
    process_unjoin_allocation_array(process, ptr);

    //free the memory
    kfree(ptr);
    

}

//sets process arguments from the params into the process structure
void process_get_arguments(struct process *process, int* argc, char** argv)
{

    *argc = process->arguments.argc;
    *argv = process->arguments.argv;
}

//this function counts the arguments
int process_count_command_arguments(struct command_argument *root_argument)
{
    struct command_argument *current = root_argument;
    int i = 0;

    //loops threw the linked list
    while (current)
    {   
        i++;
        current = current->next;
    }

    return i;
    
}


//this function injects all the arguments (command_argument) in the linked list into the process
int process_inject_arguments(struct process *process, struct command_argument *root_argument)
{
    int res = 0;
    struct command_argument *current = root_argument;
    int i = 0;
    int argc = process_count_command_arguments(root_argument);      //counts how many arguments there are

    if (argc == 0)
    {
        res = -EIO;
        goto out;
    }

    char** argv = process_malloc(process, (sizeof(const char*) * argc));    //allocates memory in the process for the argv array
    if (!argv)
    {
        res = -ENOMEM;
        goto out;
    }


    //in this while loop i go threw all the command arguments and allocate memory for them and move every single command argument in the
    // "argv" pointer/array

    /*here i loop threw the linked list of the command arguments and copy every argument into the process and add every single 
    argument to the argv array*/
    while (current)
    {

        //allocate memory für das aktuelle Argument
        char *argument_str = process_malloc(process, sizeof(current->argument));
        if (!argument_str)
        {
            res = -ENOMEM;
            goto out;
        }

        //Kopiert das Argument in den reservierten Speicher
        strncpy(argument_str, current->argument, sizeof(current->argument));
        argv[i] = argument_str;
        current = current->next;
        i++;
        
    }


    //weist die Argumente dem process (process structure) zu
    process->arguments.argc = argc;
    process->arguments.argv = argv;
    
    

out:
    return res;
}



//this function loads a process normal with the "process_load" function and then switches the process to the new loaded process
int process_load_switch(const char *filename, struct process **process)
{
    int res = process_load(filename, process);
    if (res == 0)
    {
        process_switch(*process);
    }

    return res;
}

//this function loops threw all 1024 program allocations and frees them with the "process_free" function
int process_terminate_allocations(struct process *process)
{

    for (int i = 0; i < SLOBOS_MAX_PROGRAM_ALLOCATIONS; i++)
    {
        process_free(process, process->allocations[i].ptr);
    }
    
    return 0;
}

//frees the memory of a binary program
int process_free_binary_data(struct process *process)
{

    kfree(process->ptr);
    return 0;
}

//frees the memory of a elf program
int process_free_elf_data(struct process *process)
{
    elf_close(process->elf_file);
}


//chooses between a elf and a binary program and then frees up the memory for it
int process_free_program_data(struct process *process)
{

    int res = 0;
    switch (process->filetype)
    {
    case PROCESS_FILETYPE_ELF:
        res = process_free_elf_data(process);
        break;

    case PROCESS_FILETYPE_BINARY:
        res = process_free_binary_data(process);
        break;
    default:
        res = -EINVARG;
    }

    return res;

}

//this function loops threw all processes and switches to the first true one, if there is noone, the kernel panics
void process_switch_to_any()
{

    for (int i = 0; i < SLOBOS_MAX_PROCESSES; i++)
    {
        if (processes[i])
        {
            process_switch(processes[i]);
            return;
        }
    
    }

    panic("No processes to switch to\n");
    
}

//diese Funktion entfernt den gegebenen Prozess aus dem "processes" array. Falls der aktuelle process übergeben wird, wechselt es zu einem beliebigen prozess
static void process_unlink(struct process *process)
{
    processes[process->id] = 0x00;

    //if process is the current process, it switches to a random process
    if (current_process == process)
    {
        process_switch_to_any();
    }
    
}


//brings all the functions together for terminating a process
int process_terminate(struct process* process)
{
    int res = 0;

    res = process_terminate_allocations(process);
    if (res < 0)
    {
        goto out;
    }

    res = process_free_program_data(process);
    if (res < 0)
    {
        goto out;
    }

    // Free the process stack memory.
    kfree(process->stack);
    // Free the task
    task_free(process->task);
    // Unlink the process from the process array.
    process_unlink(process);

out:
    return res;
}
