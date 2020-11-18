#ifndef KBVEC__H
#define KBVEC__H

#include <stdlib.h>
#include <assert.h>
#include "kobalt/log.h"
#include "kobalt/memory.h"

// TODO: add safe mode

struct kbvec {
    void* data;
    int size;
    int capacity;
    size_t elem_size;
};

void kbvec_new(struct kbvec* vec, size_t elem_size);
void kbvec_del(struct kbvec* vec);
void* kbvec_get(struct kbvec* vec, int idx);
void* kbvec_last(struct kbvec* vec);
void kbvec_reserve(struct kbvec* vec, int newcap);
void kbvec_resize(struct kbvec* vec, int newsize);
void kbvec_push(struct kbvec* vec, void* elem);
void kbvec_pop(struct kbvec* vec, void* elem);
void kbvec_peek(struct kbvec* vec, void* elem);
void kbvec_fill(struct kbvec* vec, void* elem);

#define kbvec_decl(TYPE, NAME) \
struct kbvec_##NAME {\
    TYPE* data;\
    int size;\
    int capacity;\
};\
void kbvec_##NAME##_new(struct kbvec_##NAME* vec);\
void kbvec_##NAME##_del(struct kbvec_##NAME* vec);\
void kbvec_##NAME##_reserve(struct kbvec_##NAME* vec, int newcap);\
void kbvec_##NAME##_resize(struct kbvec_##NAME* vec, int newsize);\
void kbvec_##NAME##_push(struct kbvec_##NAME* vec, TYPE elem);\
TYPE kbvec_##NAME##_get(struct kbvec_##NAME* vec, int idx);\
TYPE kbvec_##NAME##_first(struct kbvec_##NAME* vec);\
TYPE kbvec_##NAME##_last(struct kbvec_##NAME* vec);\
TYPE kbvec_##NAME##_pop(struct kbvec_##NAME* vec);\
void kbvec_##NAME##_fill(struct kbvec_##NAME* vec, TYPE elem);

#define kbvec_impl(TYPE, NAME)\
void kbvec_##NAME##_new(struct kbvec_##NAME* vec) {\
    vec->capacity = 0;\
    vec->size = 0;\
    vec->data = NULL;\
}\
void kbvec_##NAME##_del(struct kbvec_##NAME* vec) {\
    if (vec->data) {\
        kbfree(vec->data);\
    }\
}\
void kbvec_##NAME##_reserve(struct kbvec_##NAME* vec, int newcap) {\
    if (newcap > vec->capacity) {\
        vec->capacity = newcap;\
        vec->data = kbrealloc(vec->data, vec->capacity * sizeof(TYPE));\
    }\
}\
void kbvec_##NAME##_resize(struct kbvec_##NAME* vec, int newsize) {\
    kbvec_##NAME##_reserve(vec, newsize);\
    vec->size = newsize;\
}\
void kbvec_##NAME##_push(struct kbvec_##NAME* vec, TYPE elem) {\
    if (vec->capacity == vec->size) {\
        kbvec_##NAME##_reserve(vec, (vec->capacity == 0)? 8 : 2 * vec->capacity);\
    }\
   vec->data[vec->size] = elem;\
    ++ vec->size;\
}\
TYPE kbvec_##NAME##_pop(struct kbvec_##NAME* vec) {\
    assert(vec->size > 0);\
    -- vec->size;\
    return vec->data[vec->size];\
}\
TYPE kbvec_##NAME##_get(struct kbvec_##NAME* vec, int idx) {\
    return vec->data[idx];\
}\
TYPE kbvec_##NAME##_last(struct kbvec_##NAME* vec) {\
    return vec->data[vec->size - 1];\
}\
void kbvec_##NAME##_fill(struct kbvec_##NAME* vec, TYPE elem) {\
    for (int i = 0; i < vec->size; ++i) {\
        vec->data[i] = elem;\
    }\
}

kbvec_decl(char, char)
kbvec_decl(int, int)
kbvec_decl(char*, cstr)


#endif
