#ifndef ABLqueue__H
#define ABLqueue__H

#include <stdlib.h>
#include <assert.h>
#include "abl/log.h"
#include "abl/mem.h"

struct abl_queue {
    void* data;
    int front;
    int size;
    int capacity;
    size_t elem_size;
};

ABL_API void abl_queue_new(struct abl_queue* queue, size_t elem_size);
ABL_API void abl_queue_del(struct abl_queue* queue);
ABL_API void abl_queue_reserve(struct abl_queue* queue, int newcap);
ABL_API void abl_queue_enqueue(struct abl_queue* queue, void* elem);
ABL_API void abl_queue_dequeue(struct abl_queue* queue, void* elem);

#define abl_queue_decl(TYPE, NAME) \
struct abl_queue_##NAME {\
    TYPE* data;\
    int front;\
    int size;\
    int capacity;\
};\
ABL_API void abl_queue_##NAME##_new(struct abl_queue_##NAME* queue);\
ABL_API void abl_queue_##NAME##_del(struct abl_queue_##NAME* queue);\
ABL_API void abl_queue_##NAME##_reserve(struct abl_queue_##NAME* queue, int newcap);\
ABL_API void abl_queue_##NAME##_enqueue(struct abl_queue_##NAME* queue, TYPE elem);\
ABL_API TYPE abl_queue_##NAME##_dequeue(struct abl_queue_##NAME* queue);\

#define abl_queue_impl(TYPE, NAME)\
ABL_API void abl_queue_##NAME##_new(struct abl_queue_##NAME* queue) {\
    queue->front = 0;\
    queue->capacity = 8;\
    queue->size = 0;\
    queue->data = abl_malloc(sizeof(TYPE) * queue->capacity);\
}\
ABL_API void abl_queue_##NAME##_del(struct abl_queue_##NAME* queue) {\
    if (queue->data) {\
        abl_free(queue->data);\
    }\
}\
ABL_API void abl_queue_##NAME##_reserve(struct abl_queue_##NAME* queue, int newcap) {\
    if (newcap > queue->capacity) {\
        queue->capacity = newcap;\
        queue->data = abl_realloc(queue->data, queue->capacity * sizeof(TYPE));\
    }\
}\
ABL_API void abl_queue_##NAME##_enqueue(struct abl_queue_##NAME* queue, TYPE elem) {\
    if (queue->capacity == queue->size) {\
        int prev_cap = queue->capacity;\
        abl_queue_##NAME##_reserve(queue, 2 * queue->capacity);\
        if (queue->front + queue->size > prev_cap) {\
            memcpy((char*)queue->data + (prev_cap - 1) * sizeof(TYPE), queue->data, sizeof(TYPE));\
        }\
    }\
   queue->data[queue->size] = elem;\
   ++ queue->size;\
}\
ABL_API TYPE abl_queue_##NAME##_dequeue(struct abl_queue_##NAME* queue) {\
    assert(queue->size > 0);\
    -- queue->size;\
    return queue->data[queue->size];\
}

abl_queue_decl(int, int)

#endif
