#include "kobalt/vec.h"
#include "kobalt/memory.h"
#include <string.h>
#include <assert.h>

kbvec_impl(char, char)
kbvec_impl(int, int)
kbvec_impl(char*, cstr)

// TODO: shrink vec

void kbvec_new(struct kbvec* vec, size_t elem_size) {
    vec->capacity = 0;
    vec->size = 0;
    vec->data = NULL;
    vec->elem_size = elem_size;
}

void kbvec_del(struct kbvec* vec) {
    if (vec->data) {
        kbfree(vec->data);
    }
}

void kbvec_reserve(struct kbvec* vec, int newcap) {
    if (newcap > vec->capacity) {
        vec->capacity = newcap;
        vec->data = kbrealloc((char*)vec->data, vec->capacity * vec->elem_size);
    }
}

void kbvec_resize(struct kbvec* vec, int newsize) {
    kbvec_reserve(vec, newsize);
    vec->size = newsize;
}

void kbvec_push(struct kbvec* vec, void* elem) {
    if (vec->capacity == vec->size) {
        kbvec_reserve(vec, (vec->capacity == 0)? 8 : 2 * vec->capacity);
    }
    memcpy((char*)vec->data + vec->size * vec->elem_size , elem, vec->elem_size);
    ++ vec->size;
}

void kbvec_pop(struct kbvec* vec, void* elem) {
#if DEBUG
    assert(vec->size);
#endif
    if (elem != NULL) {
        memcpy(elem, (char*)vec->data + (vec->size - 1) * vec->elem_size, vec->elem_size);
    }
    -- vec->size;
}

void* kbvec_get(struct kbvec* vec, int idx) {
#if DEBUG
    assert(idx < vec->size);
#endif
    return (char*)vec->data + idx * vec->elem_size;
}

void* kbvec_last(struct kbvec* vec) {
    void* last = (char*)vec->data + (vec->size - 1) * vec->elem_size;
    return last;
}

void kbvec_fill(struct kbvec* vec, void* elem) {
    for (int i = 0; i < vec->size; ++i) {
        memcpy((char*)vec->data + i * vec->elem_size, elem, vec->elem_size);
    }
}
