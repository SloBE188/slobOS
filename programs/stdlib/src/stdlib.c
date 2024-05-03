#include "stdlib.h"
#include "slobos.h"


void *malloc(size_t size)
{
    return slobos_malloc(size);
}


void free(void *ptr)
{
    slobos_free(ptr);
}