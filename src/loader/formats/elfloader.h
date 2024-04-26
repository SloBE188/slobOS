#ifndef ELFLOADER_H
#define ELFLOADER_H

#include <stdint.h>
#include <stddef.h>
#include "elf.h"
#include "config.h"

struct elf_file
{
    char filename[SLOBOS_MAX_PATH];     //name of the file

    int in_memory_size;                 //size of the file

    //The physical memory address of the elf file (so it contains the whole elf file, everything)
    void* elf_memory;


    //The virtual base address of this binary is the start of it (it points to the first loadable section of this binary file)
    void* virtual_base_address;

    //The virtual end address of this binary is the end of it (it points to the last loadable section of this binary file)
    void* virtual_end_address;

    //start of the end file (but this time the physical address of it and not the virtual)
    void* physical_base_address;


    //end of the end file (but this time the physical address of it and not the virtual)
    void* physical_end_address;

};


int elf_load(const char *filename, struct elf_file **file_out);
void elf_close(struct elf_file *file);
void* elf_virtual_base(struct elf_file *file);
void* elf_virtual_end(struct elf_file *file);

void* elf_phys_base(struct elf_file *file);
void* elf_phys_end(struct elf_file *file);

void* elf_memory(struct elf_file *file);
struct elf_header *elf_header(struct elf_file *file);
struct elf32_shdr *elf_sheader(struct elf_header *header);
struct elf32_phdr *elf_pheader(struct elf_header *header);
struct elf32_phdr *elf_program_header(struct elf_header *header, int index);
struct elf32_shdr* elf_section(struct elf_header* header, int index);
void* elf_phdr_phys_addr(struct elf_file *file, struct elf32_phdr *phdr);

#endif