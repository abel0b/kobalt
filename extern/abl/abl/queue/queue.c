#include "abl/queue.h"
#include <string.h>

abl_queue_impl(int, int)

ABL_API void abl_queue_new(struct abl_queue* queue, size_t elem_size) {
    queue->front = 0;
    queue->capacity = 8;
    queue->size = 0;
    queue->elem_size = elem_size;
    queue->data = abl_malloc(elem_size * queue->capacity);
}

ABL_API void abl_queue_del(struct abl_queue* queue) {
    if (queue->data) {
        abl_free(queue->data);
    }
}

ABL_API void abl_queue_reserve(struct abl_queue* queue, int newcap) {
    if (newcap > queue->capacity) {
        queue->capacity = newcap;
        queue->data = abl_realloc(queue->data, queue->capacity * queue->elem_size);
    }
}

ABL_API void abl_queue_enqueue(struct abl_queue* queue, void* elem) {
    if (queue->size == queue->capacity && queue->size) {
        int prev_cap = queue->capacity;
        queue->capacity *= 2;
        queue->data = abl_realloc(queue->data, queue->capacity * queue->elem_size);
        if (queue->front + queue->size > prev_cap) {
            memcpy((char*)queue->data + (prev_cap - 1) * queue->elem_size, queue->data, queue->elem_size);
        }
    }
    memcpy((char*)queue->data + queue->elem_size * ((queue->front + queue->size) % queue->capacity), elem, queue->elem_size);
    ++ queue->size;
}

ABL_API void abl_queue_dequeue(struct abl_queue* queue, void* elem) {
    -- queue->size;
    memcpy(elem, (char*)queue->data + queue->elem_size * queue->front, queue->elem_size);
    queue->front = (queue->front + 1) % queue->capacity;
}
