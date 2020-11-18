#include "kobalt/queue.h"
#include <string.h>

void kbqueue_new(struct kbqueue* queue, size_t elem_size) {
    queue->front = 0;
    queue->capacity = 8;
    queue->size = 0;
    queue->elem_size = elem_size;
    queue->data = kbmalloc(elem_size * queue->capacity);
}

void kbqueue_del(struct kbqueue* queue) {
    if (queue->data) {
        kbfree(queue->data);
    }
}

void kbqueue_reserve(struct kbqueue* queue, int newcap) {
    if (newcap > queue->capacity) {
        queue->capacity = newcap;
    }
    queue->data = kbrealloc(queue->data, queue->capacity * queue->elem_size);
}

void kbqueue_enqueue(struct kbqueue* queue, void* elem) {
    if (queue->size == queue->capacity && queue->size) {
        int prev_cap = queue->capacity;
        queue->capacity *= 2;
        queue->data = kbrealloc(queue->data, queue->capacity * queue->elem_size);
        if (queue->front + queue->size > prev_cap) {
            memcpy((char*)queue->data + (prev_cap - 1) * queue->elem_size, queue->data, queue->elem_size);
        }
    }
    ++ queue->size;
    memcpy((char*)queue->data + queue->elem_size * ((queue->front + queue->size) % queue->capacity), elem, queue->elem_size);
}

void kbqueue_dequeue(struct kbqueue* queue, void* elem) {
    queue->size --;
    memcpy(elem, (char*)queue->data + queue->elem_size * queue->front, queue->elem_size);
    queue->front = (queue->front + 1) % queue->capacity;
}
