#ifndef KLDICT__H
#define KLDICT__H

#include "kobalt/kobalt.h"
#include "klbase/strstack.h"
#include <stdint.h>

struct kl_dictentry {
    char* key;
    void* value;
};

struct kl_dict {
    struct kl_str_stack key_pool;
    int numbuckets;
    struct kl_dictentry** buckets;
    int* sizes;
    int* capacities;
};

void kl_dict_new(struct kl_dict* dict);
void kl_dict_set(struct kl_dict* dict, char* key, void* value);
void* kl_dict_get(struct kl_dict* dict, char* key);
void kl_dict_display(struct kl_dict* dict);
void kl_dict_del(struct kl_dict* dict);

#endif
