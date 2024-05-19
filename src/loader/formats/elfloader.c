#include "elfloader.h"
#include "fs/file.h"
#include <stdbool.h>
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "string/string.h"
#include "memory/paging/paging.h"
#include "kernel.h"
#include "config.h"
#include "status.h"


//ELf signature Array
const char elf_signature[] = {0x7f, 'E', 'L', 'F'};


//Checks if the elf signature is right with the memcmp (compares the param with the elf_signature array)
static bool elf_valid_signature(void* buffer)
{
    return memcmp(buffer, (void*) elf_signature, sizeof(elf_signature)) == 0;
}

//checks if the file class is valid. It can be nothing, 32bit or 64 bit but because my os is in PM i only support 32bit binaries and stuff
static bool elf_valid_class(struct elf_header *header)
{
    return header->e_ident[EI_CLASS] == ELFCLASSNONE || header->e_ident[EI_CLASS] == ELFCLASS32;
}

//Checks if the encoding is valid (little endian, big endian & invalid data encoding). My elfloader will only support little endian(ELFDATA2LSB)
static bool elf_valid_encoding(struct elf_header *header)
{
    return header->e_ident[EI_DATA] == ELFDATANONE || header->e_ident[EI_DATA] == ELFDATA2LSB;
}

//The elf loader curr only support executable files so it checks if the file is a executable and gets loaded above the SLOBOS_PROGRAM_VIRTUAL_ADDRESS(0x400000)
static bool elf_is_executable(struct elf_header *header)
{
    return header->e_type == ET_EXEC && header->e_entry >= SLOBOS_PROGRAM_VIRTUAL_ADDRESS;
}

//checks if the elf file has a program header(it needs it lol)
static bool elf_has_program_header(struct elf_header* header)
{
    return header->e_phoff != 0;
}

//gives me the physical address where the elf file is loaded back
void* elf_memory(struct elf_file *file)
{
    return file->elf_memory;
}

//returns the elf header (the start of the elf file is the elf header)
struct elf_header *elf_header(struct elf_file *file)
{
    return file->elf_memory;
}


//gives the section header(i get it from the start of the elf file(header) + the section header table offset)
struct elf32_shdr *elf_sheader(struct elf_header *header)
{
    return (struct elf32_shdr*)((int)header+header->e_shoff);
}

//gives the program header(i get it from the start of the elf file(header) + the program header table offset)
struct elf32_phdr *elf_pheader(struct elf_header *header)
{
    //if there is no program header
    if (header->e_phoff == 0)
    {
        return 0;
    }
    
    //if there is one, return it
    return (struct elf32_phdr*)((int)header+header->e_phoff);
    
}

//this will return a particular program header entry based on the index provided with the "elf_pheader" function
struct elf32_phdr *elf_program_header(struct elf_header *header, int index)
{
    return &elf_pheader(header)[index];
}


//this will return a particular section header entry based on the index provided with the "elf_sheader" function
struct elf32_shdr* elf_section(struct elf_header* header, int index)
{
    return &elf_sheader(header)[index];
}

//gives physical program header address
void* elf_phdr_phys_addr(struct elf_file *file, struct elf32_phdr *phdr)
{
    return elf_memory(file)+phdr->p_offset;
}


//returns the section header string (again header + offset of the section header beacuse the first thing in a elf file ist the elf header) based on the index "e_shstrndx"
char *elf_str_table(struct elf_header *header)
{
    return (char*) header + elf_section(header, header->e_shstrndx)->sh_offset;
}


//returns the start of the elf file (first loadable section in the binary) (virtual address)
void* elf_virtual_base(struct elf_file *file)
{
    return file->virtual_base_address;
}

//returns the end of the elf file (last loadable section in the binary) (virtual address)
void* elf_virtual_end(struct elf_file *file)
{
    return file->virtual_end_address;
}

//returns the start of the elf file (first loadable section in the binary) (physical address)
void* elf_phys_base(struct elf_file *file)
{
    return file->physical_base_address;
}

//returns the end of the elf file (last loadable section in the binary) (physical address)
void* elf_phys_end(struct elf_file *file)
{
    return file->physical_end_address;
}


//puts helper functions together and checks if the elf file is valid and can be loaded!
int elf_validate_loaded(struct elf_header *header)
{
    return (elf_valid_signature(header) && elf_valid_class(header) && elf_valid_encoding(header) && elf_has_program_header(header)) ? SLOBOS_ALL_OK : -EINFORMAT;
}


// Verarbeitet den Programmheader für den Typ PT_LOAD (mein elf loader unterstützt keinen anderen typ, nur static)
int elf_process_phdr_pt_load(struct elf_file *elf_file, struct elf32_phdr *phdr)
{

    // Hier erfolgt die eigentliche Adressberechnung und Speicherzuweisung basierend auf dem Programmheader
    if (elf_file->virtual_base_address >= (void*) phdr->p_vaddr || elf_file->virtual_base_address == 0x00)
    {
        elf_file->virtual_base_address = (void*) phdr->p_vaddr;
        elf_file->physical_base_address = elf_memory(elf_file)+phdr->p_offset;
    }
    
    unsigned int end_virtual_address = phdr->p_vaddr + phdr->p_filesz;
    if (elf_file->virtual_end_address <= (void*)(end_virtual_address) || elf_file->virtual_end_address == 0x00) 
    {
        elf_file->virtual_end_address = (void*) end_virtual_address;
        elf_file->physical_end_address = elf_memory(elf_file)+phdr->p_offset+phdr->p_filesz;
    }
    

    return 0;
}


// Verarbeitet einen spezifischen Programmheader
int elf_process_pheader(struct elf_file *elf_file, struct elf32_phdr *phdr)
{
    int res = 0;
    switch (phdr->p_type)
    {
    case PT_LOAD:
        res = elf_process_phdr_pt_load(elf_file, phdr);
    break;
    }
    return res;
}


// Verarbeitet alle Programmheader der ELF-Datei und gibt jeden einzelnen program header vom elf file der funktion "elf_process_pheader" weiter, 
//welche dann jeden einzelnen programm header spezifisch verarbeitet
int elf_process_pheaders(struct elf_file *elf_file)
{
    int res = 0;
    struct elf_header *header = elf_header(elf_file);   //returns the elf header (start of a elf file)
    for (int i = 0; i < header->e_phnum ; i++)  //loops threw all the elf program headers with the e_phnum variable(stores the count of the program headers)
    {
        struct elf32_phdr *phdr = elf_program_header(header, i);        //gets every program header of the elf file given
        res = elf_process_pheader(elf_file, phdr);      //processes every single program header
        if (res < 0)
        {
            break;
        }
        
    }
    

    return res;
}


// Verarbeitet die geladene ELF-Datei
int elf_process_loaded(struct elf_file *elf_file)
{
    int res = 0;
    struct elf_header *header = elf_header(elf_file);
    res = elf_validate_loaded(header);          //checks if the elf file is supported by my elf loader
    if (res < 0)
    {
        goto out;
    }
    
    res = elf_process_pheaders(elf_file);       //processes program headers (gives control to elf_process_pheaders function which then gives it to elf_process_pheaders and then to elf_process_phdr_pt_load)
    if (res < 0)
    {
        goto out;
    }
    

out:
    return res;
}

/*Das ist die Hauptfunktion die für das Laden eines elf files zuständig ist. Sie lädt eine ELF-Datei von einer Datei.
Sie reserviert speicher auf dem heap für die elf_file struktur und dann noch für das ganze elf file. D
ann ruft sie die funktion "elf_process_loaded" auf, welche dann "elf_process_pheaders" aufruft, die dann "elf_process_pheader" aufruft und am schluss ruft elf_process_pheader dann die funktion "elf_process_phdr_pt_load" auf*/
int elf_load(const char *filename, struct elf_file **file_out)
{
    struct elf_file *elf_file = kzalloc(sizeof(struct elf_file));
    int fd = 0;

    int res = fopen(filename, "r");
    if (res <= 0)
    {
        res = -EIO;
        goto out;
    }
    
    fd = res;       //fd is the filepointer now
    struct file_stat stat;
    res = fstat(fd, &stat);     //gets the size of the elf file
    if (res < 0)
    {
        goto out;
    }
    
    elf_file->elf_memory = kzalloc(stat.filesize);      //reserviert speicher auf dem heap for the whole elf file, this allocated memory is curr not virtual, it gets after the paging_map_elf function
    res = fread(elf_file->elf_memory, stat.filesize, 1, fd);    //reading the entire file into memory
    if (res < 0)
    {
        goto out;
    }
    
    res = elf_process_loaded(elf_file); //processes a loaded elf file

    if (res < 0)
    {
        goto out;
    }
    
    *file_out = elf_file;       //sets the pointer value of the file_out param so it is the elf_file i worked with in this function

out:
    fclose(fd);
    return res;
}

//closes the elf file(frees all the memory in the heap)
void elf_close(struct elf_file *file)
{
    if (!file)
    {
        return;
    }
    
    kfree(file->elf_memory);
    kfree(file);
}

