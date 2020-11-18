#ifndef KBDICT__H
#define KBDICT__H

#include "kobalt/kobalt.h"
#include "kobalt/strstack.h"
#include <stdint.h>

struct kbdictentry {
    char* key;
    void* value;
};

struct kbdict {
    struct kbstr_stack key_pool;
    int numbuckets;
    struct kbdictentry** buckets;
    int* sizes;
    int* capacities;
};

void kbdict_new(struct kbdict* dict);
void kbdict_set(struct kbdict* dict, char* key, void* value);
void* kbdict_get(struct kbdict* dict, char* key);
void kbdict_display(struct kbdict* dict);
void kbdict_del(struct kbdict* dict);

#endif
