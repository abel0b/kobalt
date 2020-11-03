#ifndef KBOBJPOOL__H
#define KBOBJPOOL__H

#include <stdlib.h>
#include "kobalt/kobalt.h"

struct kbobjpool {
    char* data;
    size_t elem_size;
    int num_elems;
    int cap_elems;
    void (*del)(void*);
};

void kbobjpool_new(struct kbobjpool* objpool, size_t elem_size, void del(void*));
void kbobjpool_del(struct kbobjpool* objpool);
void* kbobjpool_alloc(struct kbobjpool* objpool);
void* kbobjpool_arralloc(struct kbobjpool* objpool, int n);
void* kbobjpool_pop(struct kbobjpool* objpool);
void* kbobjpool_arrpop(struct kbobjpool* objpool, int n);
void* kbobjpool_peek(struct kbobjpool* objpool);
void* kbobjpool_arrpeek(struct kbobjpool* objpool, int n);
void* kbobjpool_get(struct kbobjpool* objpool, int idx);

#endif
