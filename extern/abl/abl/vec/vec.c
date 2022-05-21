#include "abl/vec.h"
#include "abl/mem.h"
#include <string.h>
#include <assert.h>

abl_vec_impl(char, char)
abl_vec_impl(int, int)
abl_vec_impl(char*, cstr)

// TODO: shrink vec

ABL_API void abl_vec_new(struct abl_vec* vec, size_t elem_size) {
    vec->capacity = 0;
    vec->size = 0;
    vec->data = NULL;
    vec->elem_size = elem_size;
}

ABL_API void abl_vec_del(struct abl_vec* vec) {
    if (vec->data) {
        abl_free(vec->data);
    }
}

ABL_API bool abl_vec_empty(struct abl_vec* vec) {\
    return !vec->size;
}

ABL_API void abl_vec_reserve(struct abl_vec* vec, int newcap) {
    if (newcap > vec->capacity) {
        vec->capacity = newcap;
        vec->data = abl_realloc((char*)vec->data, vec->capacity * vec->elem_size);
    }
}

ABL_API void abl_vec_resize(struct abl_vec* vec, int newsize) {
    abl_vec_reserve(vec, newsize);
    vec->size = newsize;
}

ABL_API void abl_vec_push(struct abl_vec* vec, void* elem) {
    if (vec->capacity == vec->size) {
        abl_vec_reserve(vec, (vec->capacity == 0)? 8 : 2 * vec->capacity);
    }
    memcpy((char*)vec->data + vec->size * vec->elem_size , elem, vec->elem_size);
    ++ vec->size;
}

ABL_API void abl_vec_pop(struct abl_vec* vec, void* elem) {
#if DEBUG
    assert(vec->size);
#endif
    if (elem != NULL) {
        memcpy(elem, (char*)vec->data + (vec->size - 1) * vec->elem_size, vec->elem_size);
    }
    -- vec->size;
}

ABL_API void abl_vec_peek(struct abl_vec* vec, void* elem) {
#if DEBUG
    assert(vec->size);
#endif
    if (elem != NULL) {
        memcpy(elem, (char*)vec->data + (vec->size - 1) * vec->elem_size, vec->elem_size);
    }
}

ABL_API void* abl_vec_get(struct abl_vec* vec, int idx) {
#if DEBUG
    assert(idx < vec->size);
#endif
    return (char*)vec->data + idx * vec->elem_size;
}

ABL_API void* abl_vec_last(struct abl_vec* vec) {
    void* last = (char*)vec->data + (vec->size - 1) * vec->elem_size;
    return last;
}

ABL_API void abl_vec_fill(struct abl_vec* vec, void* elem) {
    for (int i = 0; i < vec->size; ++i) {
        memcpy((char*)vec->data + i * vec->elem_size, elem, vec->elem_size);
    }
}
