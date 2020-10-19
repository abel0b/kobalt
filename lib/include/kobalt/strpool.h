#ifndef KBSTRPOOL__H
#define KBSTRPOOL__H

#include "kobalt/kobalt.h"
#include "kobalt/objpool.h"
#include "kobalt/vec.h"

struct kbstrpool {
    struct kbobjpool objpool;
    struct kbvec lens;
};

void kbstrpool_new(struct kbstrpool* strpool);

char* kbstrpool_push(struct kbstrpool* strpool, char * fmt, ...);

char* kbstrpool_pop(struct kbstrpool* strpool);

char* kbstrpool_peek(struct kbstrpool* strpool);

void kbstrpool_del(struct kbstrpool* strpool);

#endif
