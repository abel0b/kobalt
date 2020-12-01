#include "kobalt/dict.h"
#include "kobalt/memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int NUMBUCKETS = 8;
static int INITBUCKETCAPACITY = 8;

void kbdict_new(struct kbdict* dict) {
    dict->numbuckets = NUMBUCKETS;
    dict->sizes = kbmalloc(sizeof(dict->sizes[0]) * dict->numbuckets);
    memset(dict->sizes, 0, sizeof(dict->sizes[0]) * dict->numbuckets);
    dict->capacities = kbmalloc(sizeof(dict->capacities[0]) * dict->numbuckets);
    memset(dict->capacities, 0, sizeof(dict->capacities[0]) * dict->numbuckets);
    dict->buckets = NULL;
    kbstr_stack_new(&dict->key_pool);
}

static unsigned long hash(char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

void kbdict_set(struct kbdict* dict, char* key, void* value) {
    unsigned long bucketid = hash(key) % dict->numbuckets;
    if (!dict->buckets) {
        dict->buckets = kbmalloc(sizeof(dict->buckets[0]) * dict->numbuckets);
    }
    if (!dict->capacities[bucketid]) {
        dict->capacities[bucketid] = INITBUCKETCAPACITY;
        dict->buckets[bucketid] = kbmalloc(sizeof(dict->buckets[0][0]) * dict->capacities[bucketid]);
    }
    else if (dict->capacities[bucketid] == dict->sizes[bucketid]) {
        dict->capacities[bucketid] *= 2;
        dict->buckets[bucketid] = kbrealloc(dict->buckets[bucketid], sizeof(dict->buckets[0][0]) * dict->capacities[bucketid]);
    }
    
    dict->buckets[bucketid][dict->sizes[bucketid]].key = kbstr_stack_push(&dict->key_pool, key);
    dict->buckets[bucketid][dict->sizes[bucketid]].value = value;
    ++ dict->sizes[bucketid];
}

void* kbdict_get(struct kbdict* dict, char* key) {
    unsigned long bucketid = hash(key) % dict->numbuckets;
    for(int i = 0; i < dict->sizes[bucketid]; ++ i) {
        if (strcmp(key, dict->buckets[bucketid][i].key) == 0) {
            return dict->buckets[bucketid][i].value;
        }
    }
    return NULL;
}

void kbdict_del(struct kbdict* dict) {
    for(int i = 0; i < dict->numbuckets; ++ i) {
        if (dict->capacities[i]) {
            kbfree(dict->buckets[i]);
        }
    }
    kbstr_stack_del(&dict->key_pool);
    kbfree(dict->buckets);
    kbfree(dict->sizes);
    kbfree(dict->capacities);
}

void kbdict_display(struct kbdict* dict) {
    for(int i = 0; i < dict->numbuckets; ++ i) {
        if (dict->sizes[i]) {
            for(int j = 0; j < dict->sizes[i]; ++ j) {
                printf("%s => %p\n", dict->buckets[i][j].key, dict->buckets[i][j].value);
            }
        }
    }
}
