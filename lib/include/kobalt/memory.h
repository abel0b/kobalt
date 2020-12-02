#ifndef KBMEMORY__H
#define KBMEMORY__H

#include "kobalt/kobalt.h"
#include <stdlib.h>

#if DEBUG
void* kbmalloc_aux(size_t size, const char* file, int line, const char* func);
void* kbrealloc_aux(void * ptr, size_t newsize, const char* file, int line, const char* func);
void kbfree_aux(void * ptr, const char* file, int line, const char* func);
#define kbmalloc(SIZE) kbmalloc_aux(SIZE, __FILE__, __LINE__, __func__)
#define kbrealloc(PTR, SIZE)  kbrealloc_aux(PTR, SIZE, __FILE__, __LINE__, __func__)
#define kbfree(PTR) kbfree_aux(PTR, __FILE__, __LINE__, __func__)
#else
void* kbmalloc_aux(size_t size);
void* kbrealloc_aux(void * ptr, size_t newsize);
void kbfree_aux(void * ptr);
#define kbmalloc kbmalloc_aux
#define kbrealloc kbrealloc_aux
#define kbfree kbfree_aux
#endif

#endif
