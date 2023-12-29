#include "paging.h"
#include "memory/heap/kheap.h"

void paging_load_directory(uint32_t* directory);

static uint32_t current_directory = 0;
struct paging_4gb_chunk* paging_new_4gb(uint8_t flags)
{
    //Creates the memory for the page directory.
    uint32_t* directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE_AND_DIRECTORY);
    int offset = 0;
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE_AND_DIRECTORY; i++)
    {
        //Creates the memory for all the page tables of the directorys created above.
        uint32_t* page_table_entry = kzalloc(sizeof(uint32_t)* PAGING_TOTAL_ENTRIES_PER_TABLE_AND_DIRECTORY);
        for (int b = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE_AND_DIRECTORY; b++)
        {
            page_table_entry[b] = (offset + (b * PAGING_PAGE_SIZE)) | flags;
        }
        offset += (PAGING_TOTAL_ENTRIES_PER_TABLE_AND_DIRECTORY * PAGING_PAGE_SIZE);
        //Each directory entry has a pointer to a single page table.
        directory[i] = (uint32_t)page_table_entry | flags | PAGING_IS_WRITEABLE;
    }


    struct paging_4gb_chunk* chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    {
        chunk_4gb->directory_entry = directory;
        return chunk_4gb;
    };
    
    
}

void paging_switch(uint32_t* directory)
{
    paging_load_directory(directory);
    current_directory = directory;
}
uint32_t* paging_4gb_chunk_get_directory(struct paging_4gb_chunk* chunk)
{
    return chunk->directory_entry;
}