#include "kobalt/objpool.h"
#include "kobalt/memory.h"
#include <assert.h>

static const size_t CHUNK_SIZE = 4192;

static void kbobjpool_chunk_new(struct kbobjpool_chunk* chunk) {
    chunk->capacity = CHUNK_SIZE;
    chunk->cursor = 0;
    chunk->data = (char*)kbmalloc(chunk->capacity);
}

static void kbobjpool_chunk_del(struct kbobjpool_chunk* chunk, size_t elem_size, void del(void*)) {
    if (del) {
        for(size_t i = 0; i * elem_size < (size_t)chunk->cursor; i++) {
            del((void*)&chunk->data[i * elem_size]);
        }
    }
    kbfree(chunk->data);
}

void kbobjpool_new(struct kbobjpool* objpool, size_t elem_size, void del(void*)) {
    assert(elem_size < CHUNK_SIZE);
    objpool->cap_chunks = 1;
    objpool->num_chunks = 1;
    objpool->elem_size = elem_size;
    objpool->chunks = kbmalloc(sizeof(struct kbobjpool_chunk));
    kbobjpool_chunk_new(&objpool->chunks[0]);
    objpool->del = del;
}

void kbobjpool_del(struct kbobjpool* objpool) {
    if (objpool->cap_chunks) {
        for(int i = 0; i < objpool->num_chunks; ++i) {
            kbobjpool_chunk_del(&objpool->chunks[i], objpool->elem_size, objpool->del);
        }
        kbfree(objpool->chunks);
    }
}

void* kbobjpool_alloc(struct kbobjpool* objpool) {
    int chunk = objpool->num_chunks - 1;
    if (objpool->elem_size > CHUNK_SIZE - objpool->chunks[chunk].cursor) {
        if (objpool->cap_chunks == objpool->num_chunks) {
            objpool->cap_chunks *= 2;
            objpool->chunks = (struct kbobjpool_chunk*) kbrealloc(objpool->chunks, objpool->cap_chunks * sizeof(struct kbobjpool_chunk));
        }
        ++ chunk;
        ++ objpool->num_chunks;
        kbobjpool_chunk_new(&objpool->chunks[chunk]);
    }

    void* obj = (void*) (objpool->chunks[chunk].data + objpool->chunks[chunk].cursor);
    objpool->chunks[chunk].cursor += objpool->elem_size;
    return obj;
}

void* kbobjpool_arralloc(struct kbobjpool* objpool, int count) {
    assert(count * objpool->elem_size < CHUNK_SIZE);
    int chunk = objpool->num_chunks - 1;
    if (objpool->elem_size * count > CHUNK_SIZE - objpool->chunks[chunk].cursor) {
        if (objpool->cap_chunks == objpool->num_chunks) {
            objpool->cap_chunks *= 2;
            objpool->chunks = (struct kbobjpool_chunk*) kbrealloc(objpool->chunks, objpool->cap_chunks * sizeof(struct kbobjpool_chunk));
        }
        ++ chunk;
        ++ objpool->num_chunks;
        kbobjpool_chunk_new(&objpool->chunks[chunk]);
    }

    void* obj = (void*) (objpool->chunks[chunk].data + objpool->chunks[chunk].cursor);
    objpool->chunks[chunk].cursor += count * objpool->elem_size;
    return obj;
}

void kbobjpool_pop(struct kbobjpool* objpool, int count) {
    int chunk = objpool->num_chunks - 1;
    assert((size_t)objpool->chunks[chunk].cursor >= count* objpool->elem_size);
    objpool->chunks[chunk].cursor -= count * objpool->elem_size;
    if (objpool->chunks[chunk].cursor == 0 && objpool->num_chunks > 1) {
        -- objpool->num_chunks;
    }
}
