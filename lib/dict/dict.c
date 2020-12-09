#include "klbase/dict.h"
#include "klbase/mem.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int NUMBUCKETS = 8;
static int INITBUCKETCAPACITY = 8;

void kl_dict_new(struct kl_dict* dict) {
    dict->numbuckets = NUMBUCKETS;
    dict->sizes = kl_malloc(sizeof(dict->sizes[0]) * dict->numbuckets);
    memset(dict->sizes, 0, sizeof(dict->sizes[0]) * dict->numbuckets);
    dict->capacities = kl_malloc(sizeof(dict->capacities[0]) * dict->numbuckets);
    memset(dict->capacities, 0, sizeof(dict->capacities[0]) * dict->numbuckets);
    dict->buckets = NULL;
    kl_str_stack_new(&dict->key_pool);
}

static unsigned long hash(char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

void kl_dict_set(struct kl_dict* dict, char* key, void* value) {
    unsigned long bucketid = hash(key) % dict->numbuckets;
    if (!dict->buckets) {
        dict->buckets = kl_malloc(sizeof(dict->buckets[0]) * dict->numbuckets);
    }
    if (!dict->capacities[bucketid]) {
        dict->capacities[bucketid] = INITBUCKETCAPACITY;
        dict->buckets[bucketid] = kl_malloc(sizeof(dict->buckets[0][0]) * dict->capacities[bucketid]);
    }
    else if (dict->capacities[bucketid] == dict->sizes[bucketid]) {
        dict->capacities[bucketid] *= 2;
        dict->buckets[bucketid] = kl_realloc(dict->buckets[bucketid], sizeof(dict->buckets[0][0]) * dict->capacities[bucketid]);
    }
    
    dict->buckets[bucketid][dict->sizes[bucketid]].key = kl_str_stack_push(&dict->key_pool, key);
    dict->buckets[bucketid][dict->sizes[bucketid]].value = value;
    ++ dict->sizes[bucketid];
}

void* kl_dict_get(struct kl_dict* dict, char* key) {
    unsigned long bucketid = hash(key) % dict->numbuckets;
    for(int i = 0; i < dict->sizes[bucketid]; ++ i) {
        if (strcmp(key, dict->buckets[bucketid][i].key) == 0) {
            return dict->buckets[bucketid][i].value;
        }
    }
    return NULL;
}

void kl_dict_del(struct kl_dict* dict) {
    for(int i = 0; i < dict->numbuckets; ++ i) {
        if (dict->capacities[i]) {
            kl_free(dict->buckets[i]);
        }
    }
    kl_str_stack_del(&dict->key_pool);
    kl_free(dict->buckets);
    kl_free(dict->sizes);
    kl_free(dict->capacities);
}

void kl_dict_display(struct kl_dict* dict) {
    for(int i = 0; i < dict->numbuckets; ++ i) {
        if (dict->sizes[i]) {
            for(int j = 0; j < dict->sizes[i]; ++ j) {
                printf("%s => %p\n", dict->buckets[i][j].key, dict->buckets[i][j].value);
            }
        }
    }
}
