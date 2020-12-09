#include "klbase/objpool.h"
#include "klbase/mem.h"
#include <assert.h>

static const size_t CHUNK_SIZE = 4192;

static void kl_objpool_chunk_new(struct kl_objpool_chunk* chunk) {
    chunk->capacity = CHUNK_SIZE;
    chunk->cursor = 0;
    chunk->data = (char*)kl_malloc(chunk->capacity);
}

static void kl_objpool_chunk_del(struct kl_objpool_chunk* chunk, size_t elem_size, void del(void*)) {
    if (del) {
        for(size_t i = 0; i * elem_size < (size_t)chunk->cursor; i++) {
            del((void*)&chunk->data[i * elem_size]);
        }
    }
    kl_free(chunk->data);
}

void kl_objpool_new(struct kl_objpool* objpool, size_t elem_size, void del(void*)) {
    assert(elem_size < CHUNK_SIZE);
    objpool->cap_chunks = 1;
    objpool->num_chunks = 1;
    objpool->elem_size = elem_size;
    objpool->chunks = kl_malloc(sizeof(struct kl_objpool_chunk));
    kl_objpool_chunk_new(&objpool->chunks[0]);
    objpool->del = del;
}

void kl_objpool_del(struct kl_objpool* objpool) {
    if (objpool->cap_chunks) {
        for(int i = 0; i < objpool->num_chunks; ++i) {
            kl_objpool_chunk_del(&objpool->chunks[i], objpool->elem_size, objpool->del);
        }
        kl_free(objpool->chunks);
    }
}

void* kl_objpool_alloc(struct kl_objpool* objpool) {
    int chunk = objpool->num_chunks - 1;
    if (objpool->elem_size > CHUNK_SIZE - objpool->chunks[chunk].cursor) {
        if (objpool->cap_chunks == objpool->num_chunks) {
            objpool->cap_chunks *= 2;
            objpool->chunks = (struct kl_objpool_chunk*) kl_realloc(objpool->chunks, objpool->cap_chunks * sizeof(struct kl_objpool_chunk));
        }
        ++ chunk;
        ++ objpool->num_chunks;
        kl_objpool_chunk_new(&objpool->chunks[chunk]);
    }

    void* obj = (void*) (objpool->chunks[chunk].data + objpool->chunks[chunk].cursor);
    objpool->chunks[chunk].cursor += objpool->elem_size;
    return obj;
}

void* kl_objpool_arralloc(struct kl_objpool* objpool, int count) {
    assert(count * objpool->elem_size < CHUNK_SIZE);
    int chunk = objpool->num_chunks - 1;
    if (objpool->elem_size * count > CHUNK_SIZE - objpool->chunks[chunk].cursor) {
        if (objpool->cap_chunks == objpool->num_chunks) {
            objpool->cap_chunks *= 2;
            objpool->chunks = (struct kl_objpool_chunk*) kl_realloc(objpool->chunks, objpool->cap_chunks * sizeof(struct kl_objpool_chunk));
        }
        ++ chunk;
        ++ objpool->num_chunks;
        kl_objpool_chunk_new(&objpool->chunks[chunk]);
    }

    void* obj = (void*) (objpool->chunks[chunk].data + objpool->chunks[chunk].cursor);
    objpool->chunks[chunk].cursor += count * objpool->elem_size;
    return obj;
}

void kl_objpool_pop(struct kl_objpool* objpool, int count) {
    int chunk = objpool->num_chunks - 1;
    assert((size_t)objpool->chunks[chunk].cursor >= count* objpool->elem_size);
    objpool->chunks[chunk].cursor -= count * objpool->elem_size;
    if (objpool->chunks[chunk].cursor == 0 && objpool->num_chunks > 1) {
        -- objpool->num_chunks;
    }
}
