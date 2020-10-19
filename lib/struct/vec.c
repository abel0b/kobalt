#include "kobalt/vec.h"
#include "kobalt/memory.h"
#include <string.h>
#include <assert.h>

void kbvec_new(struct kbvec* vec, size_t elem_size) {
    vec->cap_elems = 0;
    vec->num_elems = 0;
    vec->elems = NULL;
    vec->elem_size = elem_size;
}

void kbvec_del(struct kbvec* vec) {
    if (vec->elems) {
        kbfree(vec->elems);
    }
}

void kbvec_reserve(struct kbvec* vec, int newcap) {
    if (newcap > vec->cap_elems) {
        vec->cap_elems = newcap;
        vec->elems = (char *) kbrealloc(vec->elems, vec->cap_elems * vec->elem_size);
    }
}

void kbvec_push(struct kbvec* vec, void* elem) {
    if (vec->cap_elems == vec->num_elems) {
        kbvec_reserve(vec, (vec->cap_elems == 0)? 2 : 2 * vec->cap_elems);
    }
    memcpy(vec->elems + vec->num_elems * vec->elem_size , elem, vec->elem_size);
    ++ vec->num_elems;
}

void kbvec_pop(struct kbvec* vec, void* elem) {
#if DEBUG
    assert(vec->num_elems);
#endif
    kbvec_peek(vec, elem);
    -- vec->num_elems;
}

void kbvec_peek(struct kbvec* vec, void* elem) {
    if(elem != NULL) {
        memcpy(elem, vec->elems + (vec->num_elems - 1) * vec->elem_size, vec->elem_size);
    }
}

void* kbvec_get(struct kbvec* vec, int idx) {
#if DEBUG
    assert(idx < vec->num_elems);
#endif
    return vec->elems + idx * vec->elem_size;
}
