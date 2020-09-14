#include "kobalt/memory.h"
#include "kobalt/log.h"
#include <stdio.h>
#include <stdlib.h>

static size_t mem = 0;
static size_t mempeak = 0;
static size_t memlimit = 1e9;

static int size_max(size_t a, size_t b) {
    return (a >= b)? a : b;
}

static void * checkptr(void * ptr) {
    if (ptr == NULL) {
        kbelog("dynamic memory allocation failed");
        exit(1);
    }
    return ptr;
}

static size_t checklimit(size_t newmem) {
    if (newmem > memlimit) {
        kbelog("memory limit reached");
        exit(1);
    }
    return newmem;
}

void * kbmalloc(size_t size) {
    mem = checklimit(mem+size);
    mempeak = size_max(mempeak, mem);
    return checkptr(malloc(size));
}

void * kbrealloc(void * ptr, size_t newsize) {
    return checkptr(realloc(ptr, newsize));
}

void kbfree(void * ptr) {
    free(ptr);
}
