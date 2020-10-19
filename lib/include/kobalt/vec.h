#ifndef KBVEC__H
#define KBVEC__H

#include <stdlib.h>

struct kbvec {
    char* elems;
    int num_elems;
    int cap_elems;
    size_t elem_size;
};

void kbvec_new(struct kbvec* vec, size_t elem_size);
void kbvec_del(struct kbvec* vec);
void* kbvec_get(struct kbvec* vec, int idx);
void kbvec_reserve(struct kbvec* vec, int newcap);
void kbvec_push(struct kbvec* vec, void* elem);
void kbvec_pop(struct kbvec* vec, void* elem);
void kbvec_peek(struct kbvec* vec, void* elem);

#define kbvec_last(V) ((void*)((V)->elems + ((V)->num_elems - 1) * (V)->elem_size))

#endif
