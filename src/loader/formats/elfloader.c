#include "elfloader.h"
#include "elf.h"
#include "fs/file.h"
#include "stab.h"
#include <stdbool.h>
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "string/string.h"
#include "memory/paging/paging.h"
#include "kernel.h"
#include "config.h"
#include "status.h"


//ELf signature Array
const char *elf_signature[] = {0x7f, 'E', 'L', 'F'};


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
    return header->e_Type == ET_EXEC && header->e_entry >= SLOBOS_PROGRAM_VIRTUAL_ADDRESS;
}

//checks if the elf file has a program header(it needs it lol)
static bool elf_has_program_header(struct elf_header *header)
{
    return header->ephoff != 0;
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
    if (header->e_phoff = 0)
    {
        return = 0;
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
struct elf32_shdr *elf_sheader(struct elf_header *header, int index)
{
    return &elf_sheader(header)[index];
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
void* elf_physical_base(struct elf_file *file)
{
    return file->physical_base_address;
}

//returns the end of the elf file (last loadable section in the binary) (physical address)
void* elf_physical_end(struct elf_file *file)
{
    return file->physical_end_address;
}


//puts helper functions together and checks if the elf file is valid and can be loaded!
int elf_validate_loaded(struct elf_header *header)
{
    return (elf_valid_signature(header) && elf_valid_class(header) && elf_valid_encoding(header) && elf_has_program_header(header)) ? SLOBOS_ALL_OK : -EINVARG;
}

