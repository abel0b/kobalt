#ifndef KLMEMORY__H
#define KLMEMORY__H

#include "kobalt/kobalt.h"
#include <stdlib.h>

#if DEBUG
void* kl_malloc_aux(size_t size, const char* file, int line, const char* func);
void* kl_realloc_aux(void * ptr, size_t newsize, const char* file, int line, const char* func);
void kl_free_aux(void * ptr, const char* file, int line, const char* func);
#define kl_malloc(SIZE) kl_malloc_aux(SIZE, __FILE__, __LINE__, __func__)
#define kl_realloc(PTR, SIZE)  kl_realloc_aux(PTR, SIZE, __FILE__, __LINE__, __func__)
#define kl_free(PTR) kl_free_aux(PTR, __FILE__, __LINE__, __func__)
#else
void* kl_malloc_aux(size_t size);
void* kl_realloc_aux(void * ptr, size_t newsize);
void kl_free_aux(void * ptr);
#define kl_malloc kl_malloc_aux
#define kl_realloc kl_realloc_aux
#define kl_free kl_free_aux
#endif

#endif
