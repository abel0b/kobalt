#include "kobalt/memory.h"
#include "kobalt/log.h"
#include <stdio.h>
#include <stdlib.h>

static size_t mem = 0;
static size_t mempeak = 0;
static size_t memlimit = 1000000000;

static int size_max(size_t a, size_t b) {
    return (a >= b)? a : b;
}

static void checkptr(void* ptr, size_t size) {
    if (ptr == NULL) {
        kbelog("dynamic memory allocation of %zuB failed", size);
        exit(1);
    }
}

static size_t checklimit(size_t newmem) {
    if (newmem > memlimit) {
        kbelog("memory limit reached");
        exit(1);
    }
    return newmem;
}

#if DEBUG
void* kbmalloc_aux(size_t size, const char* file, int line, const char* func) {
    if (getenv("DEBUG_MEMORY")) {
        kbilog("mem.malloc: %zuB at %s:%d:%s", size, file, line, func);
    } 
#else
void* kbmalloc_aux(size_t size) {
#endif
    mem = checklimit(mem+size);
    mempeak = size_max(mempeak, mem);
    void* data = malloc(size);
    checkptr(data, size);
    return data;
}

#if DEBUG
void* kbrealloc_aux(void * ptr, size_t newsize, const char* file, int line, const char* func) {
    if (getenv("DEBUG_MEMORY")) {
        kbilog("mem.realloc: %p %zuB at %s:%d:%s", ptr, newsize, file, line, func);
    }

#else
void* kbrealloc_aux(void * ptr, size_t newsize) {
#endif
    void* data = realloc(ptr, newsize);
    checkptr(data, newsize);
    return data;
}

#if DEBUG
void kbfree_aux(void * ptr, const char* file, int line, const char* func) {
    if (getenv("DEBUG_MEMORY")) {
        kbilog("mem.free: %p at %s:%d:%s", ptr, file, line, func);
    }
#else
void kbfree_aux(void * ptr) {
#endif
    free(ptr);
}
