#include "kobalt/dict.h"
#include "kobalt/memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int NUMBUCKETS = 8;
static int INITBUCKETCAPACITY = 8;

void kbdict_new(struct kbdict * dict) {
    dict->numbuckets = NUMBUCKETS;
    dict->sizes = kbmalloc(sizeof(dict->sizes[0]) * dict->numbuckets);
    memset(dict->sizes, 0, sizeof(dict->sizes[0]) * dict->numbuckets);
    dict->capacities = kbmalloc(sizeof(dict->capacities[0]) * dict->numbuckets);
    memset(dict->capacities, 0, sizeof(dict->capacities[0]) * dict->numbuckets);
    dict->buckets = NULL;   
}

static int hash(struct kbdict * dict, char * key) {
    // TODO: check hash function efficiency
    int hashval = 0;
    for(; *key != '\0'; ++ key) {
        hashval += *key * 11;
    }
    return hashval % dict->numbuckets;
}

void kbdict_set(struct kbdict * dict, char * key, void * value) {
    int bucketid = hash(dict, key);
    if (!dict->buckets) dict->buckets = kbmalloc(sizeof(dict->buckets[0]) * dict->numbuckets);
    if (!dict->capacities[bucketid]) {
        dict->capacities[bucketid] = INITBUCKETCAPACITY;
        dict->buckets[bucketid] = kbmalloc(sizeof(dict->buckets[0][0]) * dict->capacities[bucketid]);
    }
    else if (dict->capacities[bucketid] == dict->sizes[bucketid]) {
        dict->capacities[bucketid] *= 2;
        dict->buckets[bucketid] = kbrealloc(dict->buckets[bucketid], sizeof(dict->buckets[0][0]) * dict->capacities[bucketid]);
    }
    
    dict->buckets[bucketid][dict->sizes[bucketid]].key = key;
    dict->buckets[bucketid][dict->sizes[bucketid]].value = value;
    ++ dict->sizes[bucketid];
}

void * kbdict_get(struct kbdict * dict, char * key) {
    int bucketid = hash(dict, key);
    for(int ii = 0; ii < dict->sizes[bucketid]; ++ ii) {
        if (strcmp(key, dict->buckets[bucketid][ii].key) == 0) {
            return dict->buckets[bucketid][ii].value;
        }
    }
    return NULL;
}

void kbdict_del(struct kbdict * dict) {
    for(int ii = 0; ii < dict->numbuckets; ++ ii) {
        if (dict->capacities[ii]) kbfree(dict->buckets[ii]);
    }
    kbfree(dict->buckets);
    kbfree(dict->sizes);
    kbfree(dict->capacities);
}

void kbdict_display(struct kbdict * dict) {
    for(int ii = 0; ii < dict->numbuckets; ++ ii) {
        if (dict->sizes[ii]) {
            for(int jj = 0; jj < dict->sizes[ii]; ++ jj) {
                printf("%s => %p\n", dict->buckets[ii][jj].key, dict->buckets[ii][jj].value);
            }
        }
    }
}
