#include "heap.h"
#include "kernel.h"
#include "status.h"
#include "memory/memory.h"
#include <stdbool.h>

/*Validates the heap table.
ptr: The pointer to the start of the heap.
end: The pointer to the end of the heap.
This function ensures that the total number of blocks calculated
from the heap size is the same as the total number of blocks in the heap table.
Returns 0 if the heap table is valid, otherwise returns -EINVARG.*/
static int heap_validate_table(void* ptr, void* end, struct heap_table* table)
{
    int res = 0;

    size_t table_size = (size_t)(end - ptr);
    size_t total_blocks = table_size / SLOBOS_HEAP_BLOCK_SIZE;
    if (table->total != total_blocks)
    {
        res = -EINVARG;
        goto out;
    }

out:
    return res;
}

/*Checks if a pointer is aligned to the heap block size.
ptr: The pointer to be checked.
The function checks if the pointer is aligned to SLOBOS_HEAP_BLOCK_SIZE.
Returns true if the pointer is aligned, false otherwise.
We need this function to prevent non 4096 byte aligned addresses being used as you aware out implementation
is build on the understanding of 4096 byte block of memory.*/
static bool heap_validate_alignment(void* ptr)
{
    return ((unsigned int)ptr % SLOBOS_HEAP_BLOCK_SIZE) == 0;
}

/*Creates the heap.
heap: The heap to be created.
ptr: The pointer to the start of the heap data.
end: The pointer to the end of the heaop data.
table: The heap table for the heap that discribes which blocks of data at ptr are free and which are not.
This funcktion creates a heap at the given memory range (ptr, end) using the privided heap table.
Returns "0" if the heap was successfully created, otherwise returns EINVARG.*/
int heap_create(struct heap* heap, void* ptr, void* end, struct heap_table* table)
{
    int res = 0;

    if (!heap_validate_alignment(ptr) || !heap_validate_alignment(end))
    {
        res = -EINVARG;
        goto out;
    }

    memset(heap, 0, sizeof(struct heap));
    heap->saddr = ptr;
    heap->table = table;

    res = heap_validate_table(ptr, end, table);
    if (res < 0)
    {
        goto out;
    }

    size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total;
    memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);

out:
    return res;
}
static uint32_t heap_align_value_to_upper(uint32_t val)
{
    if ((val % SLOBOS_HEAP_BLOCK_SIZE) == 0)
    {
        return val;
    }

    val = (val - ( val % SLOBOS_HEAP_BLOCK_SIZE));
    val += SLOBOS_HEAP_BLOCK_SIZE;
    return val;
}

static int heap_get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry)
{
    return entry & 0x0f;
}

/*This function searches the heap for a contiguous sequence of free blocks of a specified size.
The function treverses the heap's block table, seeking a sequence of HEAP_BLOCK_TABLE_ENTRY_FREE entries
that is at least as long as the total_blocks parameter
Note: If blocks in the heaphave been allocated and freed in a scattered manner, it could
be channelging to find a contiguous sequence of free blocks large enought to accomodate
the requested total_blocks. In such cases, this function may not be able to fund a suitable
starting block and would return an error.

@param heap: The heap in which to find the starting block.
@param total_blocks: The total number of cointiguous blocks required.
@return: The index of the first block in the identified sequence, or an error code
if no suitable sequence is found.*/
int heap_get_start_block(struct heap* heap, uint32_t total_blocks)
{
    struct heap_table* table = heap->table;
    int bc = 0;
    int bs = -1;

    for (size_t i = 0; i < table->total; i++)
    {
        if (heap_get_entry_type(table->entries[i]) != HEAP_BLOCK_TABLE_ENTRY_FREE)
        {
            bc = 0;
            bs = -1;
            continue;
        }

        // If this is the first block
        if (bs == -1)
        {
            bs = i;
        }
        bc++;
        if (bc == total_blocks)
        {
            break;
        }
    }

    if (bs == -1)
    {
            //returns a no memory error in the event all the momory is exhausted.
        return -ENOMEM;
    }
    
    return bs;

}

/*This takes a block number and gives you the address, its used for malloc calls.
The starting block number i spassed to the block variable in this function.
We then multiply it by the block size  and add on the starting heap data address.
This gives us the absolute address for the malloced memory which we can then pass back to the program/user.*/
void* heap_block_to_address(struct heap* heap, int block)
{
    return heap->saddr + (block * SLOBOS_HEAP_BLOCK_SIZE);
}

/*This functiuon marks the blocks allocated as taken so future malloc calls do not overricde the memory*/
void heap_mark_blocks_taken(struct heap* heap, int start_block, int total_blocks)
{
    int end_block = (start_block + total_blocks)-1;

    HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;
    if (total_blocks > 1)
    {
        entry |= HEAP_BLOCK_HAS_NEXT;
    }

    for (int i = start_block; i <= end_block; i++)
    {
        heap->table->entries[i] = entry;
        entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;
        if (i != end_block -1)
        {
            entry |= HEAP_BLOCK_HAS_NEXT;
        }
    }
}

/*Allocates a contiguous sequence of blocks in the heap.
This function attempts to allocate a number of blocks specified by total_blocks in the heap.
It first identifies a suitable starting block for the allocation unsing the heap_get_start_block
function. If nu suitable block is found, it returns NULL.
After a suitable starting block is found, it converts the block number to a memory address
using heap_block_to_address. Then it marks the allocated blocks as taken using the heap_mark_blocks_taken
function.
@param heap: The heap in which blocks are to be allocated.
@param total_blocks: The total number of contiguous blocks to allocate.
@return: The starting address of the allocated vblocks, or NULL if allocation fails.*/

void* heap_malloc_blocks(struct heap* heap, uint32_t total_blocks)
{
    void* address = 0;

    int start_block = heap_get_start_block(heap, total_blocks);
    if (start_block < 0)
    {
        goto out;
    }

    address = heap_block_to_address(heap, start_block);

    // Mark the blocks as taken
    heap_mark_blocks_taken(heap, start_block, total_blocks);

out:
    return address;
}


/*Marks a sequence of blocks in the heap as free.
This function traverses the heaps block table starting from the specified block, marking
each block it encounters as free. The traversal stops when it reaches a block that is not
marked with HEAP_BLOCK_HAS_NEXT, indicating the end of a sequence of allocated blocks.
This function is typically used to deallocate a sequence of blocks that were previosly
allocated with heap_malloc_blocks.
@param heap: The heap contaioning the blocks to be freed.
@param: starting_block: The first block in the sequence to be freed.*/
void heap_mark_blocks_free(struct heap* heap, int starting_block)
{
    struct heap_table* table = heap->table;
    for (int i = starting_block; i < (int)table->total; i++)
    {
        HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];
        table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
        if (!(entry & HEAP_BLOCK_HAS_NEXT))
        {
            break;
        }
    }
}

/*This function takes an address and converts it back into a block number in the table.*/
int heap_address_to_block(struct heap* heap, void* address)
{
    return ((int)(address - heap->saddr)) / SLOBOS_HEAP_BLOCK_SIZE;
}

/*Allocates a block of memory in the heap.
This function allocates a block of memory of a specified size in the heap.
It first aligns the requested size to the upper nearest multiple of the heap block size.
Then, it calculates the total number of blocks required to satisfy the aligned size.
Finally, it attempts to allocate this number of blocks in the heap using heap_malloc_blocks.
@param heap: The heap in which the momory is to be allocated
@param size: The size of the memory block to allocate.
@return: The starting address of the allocated memory block, or NULL if allocation fails.*/
void* heap_malloc(struct heap* heap, size_t size)
{
    size_t aligned_size = heap_align_value_to_upper(size);
    uint32_t total_blocks = aligned_size / SLOBOS_HEAP_BLOCK_SIZE;
    return heap_malloc_blocks(heap, total_blocks);
}

/*Frees a block of memory in the heap.
This function frees a block of memory in the heap that was previously allocated with heap_malloc.
It calculates the starting block number corresponding to the memory address, and then marks the
sequence of blocks starting from this block as free using heap_mark_blocks_free.
@param heap: The heap containing the memory block to be freed.
@param ptr: The starting address of the memory block to free.*/
void* heap_free(struct heap* heap, void* ptr)
{
    heap_mark_blocks_free(heap, heap_address_to_block(heap, ptr));
}
