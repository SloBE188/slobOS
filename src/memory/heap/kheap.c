#include "kheap.h"
#include "heap.h"
#include "config.h"
#include "kernel.h"
#include "memory/memory.h"
struct heap kernel_heap;
struct heap_table kernel_heap_table;

/*This function creates our kernel heap and inizializes it.*/
void kheap_init()
{
    int total_table_entries = CENTOS_HEAP_SIZE_BYTES / CENTOS_HEAP_BLOCK_SIZE;
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*)(CENTOS_HEAP_TABLE_ADDRESS);
    kernel_heap_table.total = total_table_entries;

    void* end = (void*)(CENTOS_HEAP_ADDRESS + CENTOS_HEAP_SIZE_BYTES);
    int res = heap_create(&kernel_heap, (void*)(CENTOS_HEAP_ADDRESS), end, &kernel_heap_table);
    if (res < 0)
    {
        print("Failed to create heap\n");
    }

}

/*Use this function to allocate kernel memory, its jurt like the malloc function.*/
void* kmalloc(size_t size)
{
    return heap_malloc(&kernel_heap, size);
}

/*Use this function to free kernel memory, its jurt like the free function.*/
void kfree(void* ptr)
{
    heap_free(&kernel_heap, ptr);
}
void* kzalloc(size_t size)
{
    void* ptr = kmalloc(size);
    if (ptr == 0)
    {
        return 0;
    }
    
    memset(ptr, 0x00, size);
    return ptr;
}