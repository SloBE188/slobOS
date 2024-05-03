#ifndef SLOBOS_H
#define SLOBOS_H

#include <stddef.h>

void print(const char *message);
int getkey();
void* slobos_malloc(size_t size);
void slobos_free(void *ptr);

#endif