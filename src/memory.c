#include "kobalt/memory.h"
#include <stdio.h>

static size_t mem = 0;
static size_t mempeak = 0;
static size_t memlimit = 1e9;

static int max(size_t a, size_t b) {
    return (a>=b)?a:b;
}

static void * checkptr(void * ptr) {
    if (ptr == NULL) {
        fprintf(stderr, "Memory error\n");
        exit(1);
    }
    return ptr;
}

static size_t checklimit(size_t newmem) {
    if (newmem>memlimit) {
        fprintf(stderr, "Memory limit %zu reached\n", memlimit);
        exit(1);
    }
    return newmem;
}

void * kbmalloc(size_t size) {
    mem = checklimit(mem+size);
    mempeak = max(mempeak, mem);
    return checkptr(malloc(size));
}

void * kbrealloc(void * ptr, size_t cursize, size_t newsize) {
    mem = checklimit(mem-cursize+newsize);
    return checkptr(realloc(ptr, newsize));
}

void  kbfree(void * ptr) {
    free(ptr);
}
