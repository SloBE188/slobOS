#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "io/io.h"
#include "task/task.h"

struct idt_desc idt_descriptors[SLOBOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;


//This array containts every interrupt number (represented as a index) whose value represents a pointer to the asm macro function. Array is declared in the idt.asm file.
extern void* interrupt_pointer_table[SLOBOS_TOTAL_INTERRUPTS];

//Command Array for int0x80
static ISR80H_COMMAND isr80h_commands[SLOBOS_MAX_ISR80H_COMMANDS];


//import asm functions
extern void idt_load (struct idtr_desc* ptr);
extern void int21h();
extern void no_interrupt();
extern void isr80h_wrapper();




/*This no interrupt handler will be used when their is no
associated interrupt routine for an interrupt number*/
void no_interrupt_handler()
{
    /*Send this to the PIC to acknowledge i have handled the interrupt*/
    outb(0x20,0x20); // communicating with the pic is achieved through I/O Ports (command port=0x20, data port=0x21)
}

//Divide by zero interrupt
void idt_zero()
{
    print("Divide by zero error\n");
}

//funktion wird gebraucht, um die idt table mit den interrupts zu füllen. man muss der funktion die interrupt nummer und die adresse der zuständigen ISR mitliefern.
/* in der IDT sind direkt die Adressen der ISR (Interrupt Service Routines) gespeichert. Wenn ein Interrupt auftritt, verwendet der Prozessor die IDT,
 um zu bestimmen, welche ISR aufgerufen werden soll. Diese ISR, die in Assembly geschrieben sind, fungieren als erste Anlaufstelle,
 um den aktuellen Prozessorzustand zu sichern und die Umgebung für einen sicheren Aufruf des eigentlichen Interrupt-Handlers (in C in disem file geschrieben) vorzubereiten.*/
void idt_set(int interrupt_no, void* address)
{
    struct idt_desc* desc = &idt_descriptors[interrupt_no];
    desc->offset_1 = (uint32_t) address & 0x0000ffff;
    desc->selector = KERNEL_CODE_SELECTOR;
    desc->zero = 0x00;
    desc->type_attr = 0xEE;
    desc->offset_2 = (uint32_t) address >> 16;
}

void idt_init()
{
    //memset the idt (das keine random werte drinn sind sondern nur Nullen)
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    // This is for the actual table size from the IDT
    idtr_descriptor.limit = sizeof(idt_descriptors) -1;
    /*This is the base pointer of where the idt is stored. We pass the pointer and cast it to
    uint32_t to stoire it in the descriptor*/
    idtr_descriptor.base = (uint32_t) idt_descriptors;


    //This loop sets all interrupts to point to a index in the interrupt_pointer_table array so every interrupt (i have 512) has a ISR assigned to it autimatically with my macro written in asm so i dont have to write a new ISR for every new interrupt.
    for (int i = 0; i < SLOBOS_TOTAL_INTERRUPTS; i++)
    {
        idt_set(i, interrupt_pointer_table[i]);
    }
    

    //Divide by zero interrupt (idt_zero) gets maped
    idt_set(0, idt_zero);

    /*Here i set the keyboard interrupt 0x21 (IRQ1 in the PIC) to point to the int21h
    handler which is defined in asm. The int21h handler eventually
    calls the int21h_handler C function defined above here.
    int21h is in the src/idt/idt.asm file*/
    //idt_set(0x21, int21h);



    //Interrupt 0x80 (userland) get maped. So the isr80h_wrapper function gets called when interrupt (0x80) get called through userland
    idt_set(0x80, isr80h_wrapper);

    //Load the interrupt descriptor table
    idt_load(&idtr_descriptor);

}

/*This function will simply register command functions and bind the command number (command_id) to the function pointer specified in the
command variable. After the registration is completed the command can be executed through the user process by specyfying the
command_id into the EAX reg :))))*/
void isr80h_register_command(int command_id, ISR80H_COMMAND command)
{
    if (command_id < 0 || command_id >= SLOBOS_MAX_ISR80H_COMMANDS)
    {
        panic("The OS has no more commands free (SLOBOS_MAX_ISR80H_COMMANDS)\n");
    }
    if (isr80h_commands[command_id])
    {
        panic("Youre attempting to overwrite a existing commands\n");
    }

    isr80h_commands[command_id] = command;
    
    
}


/*This function is responsible for handling all interrupt 0x80 commands.
Die Funktion überoprüft zuerst, ob der command (index davon) gültig ist, dann wird die richtige funktion
für den interrupt (systemaufruf weil es aus dem userland kommt) command_func zugewiesen, welche dann bearbeitet werden kann von der 
funktion dafür.*/
void* isr80h_handle_command(int command, struct interrupt_frame* frame)
{
    void* result = 0;

    if(command < 0 || command >= SLOBOS_MAX_ISR80H_COMMANDS)
    {
        // Invalid command
        return 0;
    }

    ISR80H_COMMAND command_func = isr80h_commands[command];
    if (!command_func)
    {
        return 0;
    }

    result = command_func(frame);
    return result;
}


/*
int command: this is the command specified in the EAX register of the user process before the process called interrupt 80.
This command represents what the kernerl should do, for example 1 represents printing a message to the screen
frame: the interrupt frame that points to the memory containing the user processses segment and general purpose registers.

So the function perfroms following steps:
1. it switches to the kernel page tables so that the memory is viewed from the kernels perspective.
2. then it saves the registers from the the currently running task(current_task) und speichert sie mit der task_current_save_state in die struktur interrupt_frame
was dem kernel zugriff darauf gibt.
3. dann callt es die isr80h_handle_command funktion welche dafür verantwortlich ist den command im EAX register (int command) auszuführen.
4. am ende wird das page directory mit task_page zurück zum page directory des tasks vom user process welcher den interrupt 0x80 invoked hat gewechselt*/
void* isr80h_handler(int command, struct interrupt_frame* frame)
{
    void* res = 0;
    kernel_page();
    task_current_save_state(frame);
    res = isr80h_handle_command(command, frame);
    task_page();
    return res;
}


void interrupt_handler(int interrupt, struct interrupt_frame *frame)
{

    outb(0x20, 0x20);
}