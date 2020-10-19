#include "kobalt/objpool.h"
#include "kobalt/memory.h"

void kbobjpool_new(struct kbobjpool* objpool, size_t elem_size, void del(void*)) {
    objpool->cap_elems = 0;
    objpool->num_elems = 0;
    objpool->elem_size = elem_size;
    objpool->data = NULL;
    objpool->del = del;
}

void kbobjpool_del(struct kbobjpool* objpool) {
    if (objpool->cap_elems) {
        if (objpool->del) {
            for(int i = 0; i < objpool->num_elems; ++i) {
                objpool->del(objpool->data + i * objpool->elem_size);
            }
        }
        kbfree(objpool->data);
    }
}

void* kbobjpool_alloc(struct kbobjpool* objpool) {
    if (objpool->cap_elems == objpool->num_elems) {
        objpool->cap_elems = (objpool->cap_elems == 0)? 16 : (2 * objpool->cap_elems);
        objpool->data = kbrealloc(objpool->data, objpool->elem_size * objpool->cap_elems);
    }

    void* obj = (void*) (objpool->data + objpool->elem_size * objpool->num_elems);
    objpool->num_elems ++;
    return obj;
}

void* kbobjpool_arralloc(struct kbobjpool* objpool, int n) {
    while (objpool->cap_elems - objpool->num_elems < n) {
        objpool->cap_elems = (objpool->cap_elems == 0)? 16 : 2 * objpool->cap_elems;
        objpool->data = kbrealloc(objpool->data, objpool->elem_size * objpool->cap_elems);
    }

    void* obj = (void*) (objpool->data + objpool->elem_size * objpool->num_elems);
    objpool->num_elems += n;
    return obj;
}

void* kbobjpool_pop(struct kbobjpool* objpool) {
    return kbobjpool_arrpop(objpool, 1);
}

void* kbobjpool_peek(struct kbobjpool* objpool) {
    return kbobjpool_arrpeek(objpool, 1);
}

void* kbobjpool_arrpop(struct kbobjpool* objpool, int n) {
    void* obj = kbobjpool_arrpeek(objpool, n);
    objpool->num_elems -= n;
    return obj;
}

void* kbobjpool_arrpeek(struct kbobjpool* objpool, int n) {
    void* obj = (void*) (objpool->data + objpool->elem_size * (objpool->num_elems - n));
    return obj;
}
