#ifndef KBMEMORY__H
#define KBMEMORY__H

#include <stdlib.h>

void * kbmalloc(size_t size);
void * kbrealloc(void * ptr, size_t size);
void kbfree(void * ptr);

#endif
