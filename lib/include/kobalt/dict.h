#ifndef KBDICT__H
#define KBDICT__H

#include "kobalt/kobalt.h"
#include <stdint.h>

struct kbdictentry {
    uintptr_t key;
    void* value;
};

struct kbdict {
    int numbuckets;
    struct kbdictentry** buckets;
    int* sizes;
    int* capacities;
};

void kbdict_new(struct kbdict* dict);
void kbdict_set(struct kbdict* dict, uintptr_t key, void* value);
void* kbdict_get(struct kbdict* dict, uintptr_t key);
void kbdict_display(struct kbdict* dict);
void kbdict_del(struct kbdict* dict);

#endif
