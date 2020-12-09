#include "klbase/vec.h"
#include "klbase/mem.h"
#include <string.h>
#include <assert.h>

kl_vec_impl(char, char)
kl_vec_impl(int, int)
kl_vec_impl(char*, cstr)

// TODO: shrink vec

void kl_vec_new(struct kl_vec* vec, size_t elem_size) {
    vec->capacity = 0;
    vec->size = 0;
    vec->data = NULL;
    vec->elem_size = elem_size;
}

void kl_vec_del(struct kl_vec* vec) {
    if (vec->data) {
        kl_free(vec->data);
    }
}

void kl_vec_reserve(struct kl_vec* vec, int newcap) {
    if (newcap > vec->capacity) {
        vec->capacity = newcap;
        vec->data = kl_realloc((char*)vec->data, vec->capacity * vec->elem_size);
    }
}

void kl_vec_resize(struct kl_vec* vec, int newsize) {
    kl_vec_reserve(vec, newsize);
    vec->size = newsize;
}

void kl_vec_push(struct kl_vec* vec, void* elem) {
    if (vec->capacity == vec->size) {
        kl_vec_reserve(vec, (vec->capacity == 0)? 8 : 2 * vec->capacity);
    }
    memcpy((char*)vec->data + vec->size * vec->elem_size , elem, vec->elem_size);
    ++ vec->size;
}

void kl_vec_pop(struct kl_vec* vec, void* elem) {
#if DEBUG
    assert(vec->size);
#endif
    if (elem != NULL) {
        memcpy(elem, (char*)vec->data + (vec->size - 1) * vec->elem_size, vec->elem_size);
    }
    -- vec->size;
}

void* kl_vec_get(struct kl_vec* vec, int idx) {
#if DEBUG
    assert(idx < vec->size);
#endif
    return (char*)vec->data + idx * vec->elem_size;
}

void* kl_vec_last(struct kl_vec* vec) {
    void* last = (char*)vec->data + (vec->size - 1) * vec->elem_size;
    return last;
}

void kl_vec_fill(struct kl_vec* vec, void* elem) {
    for (int i = 0; i < vec->size; ++i) {
        memcpy((char*)vec->data + i * vec->elem_size, elem, vec->elem_size);
    }
}
