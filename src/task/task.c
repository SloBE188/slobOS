#include "task.h"
#include "kernel.h"
#include "status.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "process.h"
#include "idt/idt.h"
#include "string/string.h"
#include "memory/paging/paging.h"
#include "loader/formats/elfloader.h"

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
        current_task = task;
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

    // Initialisiere Register (IP, SS, CS und ESP) für den Task.    
    task->registers.ip = SLOBOS_PROGRAM_VIRTUAL_ADDRESS;
    if (process->filetype == PROCESS_FILETYPE_ELF)
    {
        task->registers.ip = elf_header(process->elf_file)->e_entry;    //set the IP to the start of the elf file which is in the elf header (e_entry)
    }
    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.cs = USER_CODE_SEGMENT;
    task->registers.esp = SLOBOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START;
    task->process = process;
    

    return 0;
}


/*this functions sets the current task to the task i pass in as a parameter and then switches the page directory to the one 
from that task. after the page directory is switched, the CPU will see the system memory (adressraum) as the task should see it.
*/
int task_switch(struct task* task)
{
    current_task = task;
    paging_switch(task->page_directory);

    return 0;
}



/*this function serves as the point where the os transitions from kernel mode to user mode to the first user process. its simply the go to function
for starting the first activity that the user want to run (task).
so it switches to the first task in the linked list and executes it*/
void task_run_first_ever_task()
{

    //check if there is no curr task
    if (!current_task)
    {
        panic("task_run_first_ever_task(): NO CURRENT TASK EXISTS BROO WTF\\n");
    }

    //switch to the first task in the linked list
    task_switch(task_head);

    //transition to user mode
    task_return(&task_head->registers);
    
}


/*this function starts by switching all the segment/selector registers to point to user land and the calls task_switch to switch the
current page tables to the current task.
later this function gets used during interrupt routines to allow me to switch from the kernel context back to user space context.
this will be essential because sometimes i need to switch the processor to the kernel page tables to access kernel memory(z.B. during a interrupt)
then i can switch afterwards back to the task page tables with this function. this function can only be used in ring 0. */
int task_page()
{
    user_registers();
    task_switch(current_task);

    return 0;
}

/*this function is a task handling function that saves the salvaged user space regs from the task structure to the interrupt_frame structure
thus saving the state from a task for easy access for the kernel because the kernel can easy access the interrupt_frame structure*/
void task_save_state(struct task *task, struct interrupt_frame *frame)
{
    task->registers.ip = frame->ip;
    task->registers.cs = frame->cs;
    task->registers.flags = frame->flags;
    task->registers.esp = frame->esp;
    task->registers.ss = frame->ss;
    task->registers.eax = frame->eax;
    task->registers.ebp = frame->ebp;
    task->registers.ebx = frame->ebx;
    task->registers.ecx = frame->ecx;
    task->registers.edi = frame->edi;
    task->registers.edx = frame->edx;
    task->registers.esi = frame->esi;
}


//this function is responsible for saving the state of the currently running task (current task = last running task)
void task_current_save_state(struct interrupt_frame *frame)
{

    if (!task_current)
    {
        panic("No current task to save\n");
    }
    
    struct task *task = task_current();
    task_save_state(task, frame);

}


// Definiert eine Funktion, die einen String aus dem Adressraum eines Tasks in den physischen Speicher (kernel adressraum, da der kernel adressraum 
//l inear zum physischen speicher gemappt ist bis 0x40000(da beginnt userland)) kopiert.
// task: Zeiger auf die Task-Struktur, von der kopiert wird.
// virtual: Zeiger auf den Anfang der virtuellen Adresse im Task, von der kopiert wird.
// phys: Zeiger auf den Anfang des physischen Speicherbereichs(kernel adressrauim), in den kopiert wird.
// max: Maximale Anzahl der zu kopierenden Bytes.
int copy_string_from_task(struct task* task, void* virtual, void* phys, int max)
{
    // Überprüft, ob die maximale Größe den Wert einer Seitengröße überschreitet.
    if (max >= PAGING_PAGE_SIZE)
    {
        return -EINVARG; // Gibt einen Fehler zurück, wenn 'max' zu groß ist.
    }

    int res = 0; // Variable für das Ergebnis.
    // Reserviert Speicherplatz für einen temporären Buffer, der den zu kopierenden String enthält.
    char* tmp = kzalloc(max);
    // Überprüft, ob die Speicherreservierung erfolgreich war.
    if (!tmp)
    {
        res = -ENOMEM; // Setzt den Fehlercode, wenn die Speicherreservierung fehlschlägt.
        goto out; // Springt zum Ende der Funktion.
    }

    // Ruft das aktuelle Page directory des Tasks ab.
    uint32_t* task_directory = task->page_directory->directory_entry;
    // Speichert den aktuellen Paging-Eintrag für die temporäre Adresse.
    uint32_t old_entry = paging_get(task_directory, tmp);

    // Mappt die temporäre Adresse auf sich selbst im Task-Adressraum mit Schreib- und Leseberechtigung.
    paging_map(task->page_directory, tmp, tmp, PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    // Wechselt zum Seitenverzeichnis des Tasks.
    paging_switch(task->page_directory);
    // Kopiert den String von der virtuellen Adresse in den temporären Puffer.
    strncpy(tmp, virtual, max);
    // Wechselt zurück zum Kernel Page Directory.
    kernel_page();

    // Stellt den ursprünglichen Page directory wieder her.
    res = paging_set(task_directory, tmp, old_entry);
    if (res < 0) // Überprüft auf Fehler.
    {
        res = -EIO; // Setzt den Fehlercode bei einem Paging-Fehler.
        goto out_free; // Springt zur Freigabe des Speichers.
    }

    // Kopiert den String aus dem temporären Puffer in den physischen Speicher.
    strncpy(phys, tmp, max);

out_free:
    kfree(tmp); // Gibt den reservierten Speicher frei.

out:
    return res; // Gibt das Ergebnis zurück.
}


/*I need a way to be able to switch to any loaded tasks page tables so that the processor
can see memory from a particular tasks prespective.
This function switches all the segmekt selector registers to user space ones and the switches the current page directory
to the tasks page directory thus forcing the processor to see memory as the provided task would*/
int task_page_task(struct task* task)
{
    user_registers();
    paging_switch(task->page_directory);
    return 0;
}

/*This function will be used to aso an example pull the words 30 and 20 from the tasks stack into kernel space*/
void* task_get_stack_item(struct task* task, int index)
{
    void* result = 0;
    uint32_t* sp_ptr = (uint32_t*) task->registers.esp;
    //Switch the processor to the given tasks page directory
    task_page_task(task);
    result = (void*) sp_ptr[index];     //the stack pointer from the tasks stack where as an example 20 and 30 is stored downwords (stack grows like that) at the "index" location in the stack
    //Switch back to the kernel page
    kernel_page();
    return result;
}


//returns the physical address of a virtual address in a task
void *task_virtual_address_to_physical(struct task *task, void *virtual_address)
{
    void *physical_address = 0;
    physical_address = paging_get_physical_address(task->page_directory->directory_entry, virtual_address);
    return physical_address;
}