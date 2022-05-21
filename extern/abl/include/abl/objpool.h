#ifndef ABLOBJPOOL__H
#define ABLOBJPOOL__H

#include <stdlib.h>
#include "abl/vec.h"
#include "abl/defs.h"

struct abl_objpool_chunk {
    char* data;
    size_t cursor;
    size_t capacity;
};

struct abl_objpool {
    struct abl_objpool_chunk* chunks;
    int num_chunks;
    int cap_chunks;
    size_t elem_size;
    void (*del)(void*);
};

ABL_API void abl_objpool_new(struct abl_objpool* objpool, size_t elem_size, void del(void*));
ABL_API void abl_objpool_del(struct abl_objpool* objpool);
ABL_API void* abl_objpool_alloc(struct abl_objpool* objpool);
ABL_API void* abl_objpool_arralloc(struct abl_objpool* objpool, int n);
ABL_API void abl_objpool_pop(struct abl_objpool* objpool, int size);

#endif
