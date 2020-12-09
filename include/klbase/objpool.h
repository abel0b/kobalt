#ifndef KLOBJPOOL__H
#define KLOBJPOOL__H

#include <stdlib.h>
#include "kobalt/kobalt.h"
#include "klbase/vec.h"

struct kl_objpool_chunk {
    char* data;
    size_t cursor;
    size_t capacity;
};

struct kl_objpool {
    struct kl_objpool_chunk* chunks;
    int num_chunks;
    int cap_chunks;
    size_t elem_size;
    void (*del)(void*);
};

void kl_objpool_new(struct kl_objpool* objpool, size_t elem_size, void del(void*));
void kl_objpool_del(struct kl_objpool* objpool);
void* kl_objpool_alloc(struct kl_objpool* objpool);
void* kl_objpool_arralloc(struct kl_objpool* objpool, int n);
void kl_objpool_pop(struct kl_objpool* objpool, int size);

#endif
