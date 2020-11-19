#ifndef KBOBJPOOL__H
#define KBOBJPOOL__H

#include <stdlib.h>
#include "kobalt/kobalt.h"
#include "kobalt/vec.h"

struct kbobjpool_chunk {
    char* data;
    size_t cursor;
    size_t capacity;
};

struct kbobjpool {
    struct kbobjpool_chunk* chunks;
    int num_chunks;
    int cap_chunks;
    size_t elem_size;
    void (*del)(void*);
};

void kbobjpool_new(struct kbobjpool* objpool, size_t elem_size, void del(void*));
void kbobjpool_del(struct kbobjpool* objpool);
void* kbobjpool_alloc(struct kbobjpool* objpool);
void* kbobjpool_arralloc(struct kbobjpool* objpool, int n);
void kbobjpool_pop(struct kbobjpool* objpool, int size);

#endif
