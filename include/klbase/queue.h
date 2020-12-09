#ifndef KLqueue__H
#define KLqueue__H

#include <stdlib.h>
#include <assert.h>
#include "klbase/log.h"
#include "klbase/mem.h"

struct kl_queue {
    void* data;
    int front;
    int size;
    int capacity;
    size_t elem_size;
};

void kl_queue_new(struct kl_queue* queue, size_t elem_size);
void kl_queue_del(struct kl_queue* queue);
void kl_queue_reserve(struct kl_queue* queue, int newcap);
void kl_queue_enqueue(struct kl_queue* queue, void* elem);
void kl_queue_dequeue(struct kl_queue* queue, void* elem);

#define kl_queue_decl(TYPE, NAME) \
struct kl_queue_##NAME {\
    TYPE* data;\
    int front;\
    int size;\
    int capacity;\
};\
void kl_queue_##NAME##_new(struct kl_queue_##NAME* queue);\
void kl_queue_##NAME##_del(struct kl_queue_##NAME* queue);\
void kl_queue_##NAME##_reserve(struct kl_queue_##NAME* queue, int newcap);\
void kl_queue_##NAME##_enqueue(struct kl_queue_##NAME* queue, TYPE elem);\
TYPE kl_queue_##NAME##_dequeue(struct kl_queue_##NAME* queue);\

#define kl_queue_impl(TYPE, NAME)\
void kl_queue_##NAME##_new(struct kl_queue_##NAME* queue) {\
    queue->front = 0;\
    queue->capacity = 8;\
    queue->size = 0;\
    queue->data = kl_malloc(sizeof(TYPE) * queue->capacity);\
}\
void kl_queue_##NAME##_del(struct kl_queue_##NAME* queue) {\
    if (queue->data) {\
        kl_free(queue->data);\
    }\
}\
void kl_queue_##NAME##_reserve(struct kl_queue_##NAME* queue, int newcap) {\
    if (newcap > queue->capacity) {\
        queue->capacity = newcap;\
        queue->data = kl_realloc(queue->data, queue->capacity * sizeof(TYPE));\
    }\
}\
void kl_queue_##NAME##_enqueue(struct kl_queue_##NAME* queue, TYPE elem) {\
    if (queue->capacity == queue->size) {\
        int prev_cap = queue->capacity;\
        kl_queue_##NAME##_reserve(queue, 2 * queue->capacity);\
        if (queue->front + queue->size > prev_cap) {\
            memcpy((char*)queue->data + (prev_cap - 1) * sizeof(TYPE), queue->data, sizeof(TYPE));\
        }\
    }\
   queue->data[queue->size] = elem;\
   ++ queue->size;\
}\
TYPE kl_queue_##NAME##_dequeue(struct kl_queue_##NAME* queue) {\
    assert(queue->size > 0);\
    -- queue->size;\
    return queue->data[queue->size];\
}

kl_queue_decl(int, int)

#endif
