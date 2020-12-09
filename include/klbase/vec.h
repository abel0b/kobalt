#ifndef KLVEC__H
#define KLVEC__H

#include <stdlib.h>
#include <assert.h>
#include "klbase/log.h"
#include "klbase/mem.h"

// TODO: add safe mode

struct kl_vec {
    void* data;
    int size;
    int capacity;
    size_t elem_size;
};

void kl_vec_new(struct kl_vec* vec, size_t elem_size);
void kl_vec_del(struct kl_vec* vec);
void* kl_vec_get(struct kl_vec* vec, int idx);
void* kl_vec_last(struct kl_vec* vec);
void kl_vec_reserve(struct kl_vec* vec, int newcap);
void kl_vec_resize(struct kl_vec* vec, int newsize);
void kl_vec_push(struct kl_vec* vec, void* elem);
void kl_vec_pop(struct kl_vec* vec, void* elem);
void kl_vec_peek(struct kl_vec* vec, void* elem);
void kl_vec_fill(struct kl_vec* vec, void* elem);

#define kl_vec_decl(TYPE, NAME) \
struct kl_vec_##NAME {\
    TYPE* data;\
    int size;\
    int capacity;\
};\
void kl_vec_##NAME##_new(struct kl_vec_##NAME* vec);\
void kl_vec_##NAME##_del(struct kl_vec_##NAME* vec);\
void kl_vec_##NAME##_reserve(struct kl_vec_##NAME* vec, int newcap);\
void kl_vec_##NAME##_resize(struct kl_vec_##NAME* vec, int newsize);\
void kl_vec_##NAME##_push(struct kl_vec_##NAME* vec, TYPE elem);\
TYPE kl_vec_##NAME##_get(struct kl_vec_##NAME* vec, int idx);\
TYPE kl_vec_##NAME##_first(struct kl_vec_##NAME* vec);\
TYPE kl_vec_##NAME##_last(struct kl_vec_##NAME* vec);\
TYPE kl_vec_##NAME##_pop(struct kl_vec_##NAME* vec);\
void kl_vec_##NAME##_fill(struct kl_vec_##NAME* vec, TYPE elem);

#define kl_vec_impl(TYPE, NAME)\
void kl_vec_##NAME##_new(struct kl_vec_##NAME* vec) {\
    vec->capacity = 0;\
    vec->size = 0;\
    vec->data = NULL;\
}\
void kl_vec_##NAME##_del(struct kl_vec_##NAME* vec) {\
    if (vec->data) {\
        kl_free(vec->data);\
    }\
}\
void kl_vec_##NAME##_reserve(struct kl_vec_##NAME* vec, int newcap) {\
    if (newcap > vec->capacity) {\
        vec->capacity = newcap;\
        vec->data = kl_realloc(vec->data, vec->capacity * sizeof(TYPE));\
    }\
}\
void kl_vec_##NAME##_resize(struct kl_vec_##NAME* vec, int newsize) {\
    kl_vec_##NAME##_reserve(vec, newsize);\
    vec->size = newsize;\
}\
void kl_vec_##NAME##_push(struct kl_vec_##NAME* vec, TYPE elem) {\
    if (vec->capacity == vec->size) {\
        kl_vec_##NAME##_reserve(vec, (vec->capacity == 0)? 8 : 2 * vec->capacity);\
    }\
   vec->data[vec->size] = elem;\
    ++ vec->size;\
}\
TYPE kl_vec_##NAME##_pop(struct kl_vec_##NAME* vec) {\
    assert(vec->size > 0);\
    -- vec->size;\
    return vec->data[vec->size];\
}\
TYPE kl_vec_##NAME##_get(struct kl_vec_##NAME* vec, int idx) {\
    return vec->data[idx];\
}\
TYPE kl_vec_##NAME##_last(struct kl_vec_##NAME* vec) {\
    return vec->data[vec->size - 1];\
}\
void kl_vec_##NAME##_fill(struct kl_vec_##NAME* vec, TYPE elem) {\
    for (int i = 0; i < vec->size; ++i) {\
        vec->data[i] = elem;\
    }\
}

kl_vec_decl(char, char)
kl_vec_decl(int, int)
kl_vec_decl(char*, cstr)


#endif
