#include "elf.h"


//Returns the entry of a elf file as a pointer
void* elf_get_entry_ptr(struct elf_header *elf_header)
{
    return (void*) elf_header->e_entry;
}


//returns entry of a elf file
uint32_t elf_get_entry_(struct elf_header *elf_header)
{
    return elf_header->e_entry;
}
