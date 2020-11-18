#ifndef KBqueue__H
#define KBqueue__H

#include <stdlib.h>
#include <assert.h>
#include "kobalt/log.h"
#include "kobalt/memory.h"

struct kbqueue {
    void* data;
    int front;
    int size;
    int capacity;
    size_t elem_size;
};

void kbqueue_new(struct kbqueue* queue, size_t elem_size);
void kbqueue_del(struct kbqueue* queue);
void kbqueue_reserve(struct kbqueue* queue, int newcap);
void kbqueue_enqueue(struct kbqueue* queue, void* elem);
void kbqueue_dequeue(struct kbqueue* queue, void* elem);

#define kbqueue_decl(TYPE, NAME) \
struct kbqueue_##NAME {\
    TYPE* data;\
    int size;\
    int capacity;\
};\
void kbqueue_##NAME##_new(struct kbqueue_##NAME* queue);\
void kbqueue_##NAME##_del(struct kbqueue_##NAME* queue);\
void kbqueue_##NAME##_reserve(struct kbqueue_##NAME* queue, int newcap);\
void kbqueue_##NAME##_enqueue(struct kbqueue_##NAME* queue, TYPE elem);\
TYPE kbqueue_##NAME##_dequeue(struct kbqueue_##NAME* queue, int idx);\

#define kbqueue_impl(TYPE, NAME)\
void kbqueue_##NAME##_new(struct kbqueue_##NAME* queue) {\
    queue->capacity = 0;\
    queue->size = 0;\
    queue->data = NULL;\
}\
void kbqueue_##NAME##_del(struct kbqueue_##NAME* queue) {\
    if (queue->data) {\
        kbfree(queue->data);\
    }\
}\
void kbqueue_##NAME##_reserve(struct kbqueue_##NAME* queue, int newcap) {\
    if (newcap > queue->capacity) {\
        queue->capacity = newcap;\
        queue->data = kbrealloc(queue->data, queue->capacity * sizeof(TYPE));\
    }\
}\
void kbqueue_##NAME##_enqueue(struct kbqueue_##NAME* queue, TYPE elem) {\
    if (queue->capacity == queue->size) {\
        kbqueue_##NAME##_reserve(queue, (queue->capacity == 0)? 8 : 2 * queue->capacity);\
    }\
   queue->data[queue->size] = elem;\
    ++ queue->size;\
}\
TYPE kbqueue_##NAME##_dequeue(struct kbqueue_##NAME* queue) {\
    assert(queue->size > 0);\
    -- queue->size;\
    return queue->data[queue->size];\
}

#endif
