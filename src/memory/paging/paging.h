#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


//bit patterns for setting the flags in the Page Directory and in the Page Table
#define PAGING_CACHE_DISABLED  0b00010000
#define PAGING_WRITE_THROUGH   0b00001000
#define PAGING_ACCESS_FROM_ALL 0b00000100
#define PAGING_IS_WRITEABLE    0b00000010
#define PAGING_IS_PRESENT      0b00000001


#define PAGING_TOTAL_ENTRIES_PER_TABLE 1024     //number of entries in each page directory and page table
#define PAGING_PAGE_SIZE 4096


//structure that represents a 4GB chunk of memory. 
struct paging_4gb_chunk
{
    uint32_t* directory_entry;      //a pointer to the first entry of the page directory used to map this 4gb chunk
};

struct paging_4gb_chunk* paging_new_4gb(uint8_t flags);     //create a new 4gb chunk wioth specified flags
void paging_switch(uint32_t* directory);                    //to switch to a diffrent page directory
void enable_paging();                                       //enable paging

int paging_set(uint32_t* directory, void* virt, uint32_t val);
bool paging_is_aligned(void* addr);

uint32_t* paging_4gb_chunk_get_directory(struct paging_4gb_chunk* chunk);   //to retrieve the page directory associated with a 4gb chunk
void paging_free_4gb(struct paging_4gb_chunk* chunk);

#endif