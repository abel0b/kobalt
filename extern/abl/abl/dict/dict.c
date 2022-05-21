#include "abl/dict.h"
#include "abl/mem.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int NUMBUCKETS = 8;
static int INITBUCKETCAPACITY = 8;

ABL_API void abl_dict_new(struct abl_dict* dict) {
    dict->numbuckets = NUMBUCKETS;
    dict->sizes = abl_malloc(sizeof(dict->sizes[0]) * dict->numbuckets);
    memset(dict->sizes, 0, sizeof(dict->sizes[0]) * dict->numbuckets);
    dict->capacities = abl_malloc(sizeof(dict->capacities[0]) * dict->numbuckets);
    memset(dict->capacities, 0, sizeof(dict->capacities[0]) * dict->numbuckets);
    dict->buckets = NULL;
    abl_str_stack_new(&dict->key_pool);
}

static unsigned long hash(char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

ABL_API void abl_dict_set(struct abl_dict* dict, char* key, void* value) {
    unsigned long bucketid = hash(key) % dict->numbuckets;
    if (!dict->buckets) {
        dict->buckets = abl_malloc(sizeof(dict->buckets[0]) * dict->numbuckets);
    }
    if (!dict->capacities[bucketid]) {
        dict->capacities[bucketid] = INITBUCKETCAPACITY;
        dict->buckets[bucketid] = abl_malloc(sizeof(dict->buckets[0][0]) * dict->capacities[bucketid]);
    }
    else if (dict->capacities[bucketid] == dict->sizes[bucketid]) {
        dict->capacities[bucketid] *= 2;
        dict->buckets[bucketid] = abl_realloc(dict->buckets[bucketid], sizeof(dict->buckets[0][0]) * dict->capacities[bucketid]);
    }

    for(int i = 0; i < dict->sizes[bucketid]; ++i) {
        if(strcmp(dict->buckets[bucketid][i].key, key) == 0) { 
    	    dict->buckets[bucketid][i].value = value;
	    return;
	}
    }
    
    dict->buckets[bucketid][dict->sizes[bucketid]].key = abl_str_stack_push(&dict->key_pool, key);
    dict->buckets[bucketid][dict->sizes[bucketid]].value = value;
    ++ dict->sizes[bucketid];
}

ABL_API void* abl_dict_get(struct abl_dict* dict, char* key) {
    unsigned long bucketid = hash(key) % dict->numbuckets;
    for(int i = 0; i < dict->sizes[bucketid]; ++ i) {
        if (strcmp(key, dict->buckets[bucketid][i].key) == 0) {
            return dict->buckets[bucketid][i].value;
        }
    }
    return NULL;
}

ABL_API void abl_dict_del(struct abl_dict* dict) {
    for(int i = 0; i < dict->numbuckets; ++ i) {
        if (dict->capacities[i]) {
            abl_free(dict->buckets[i]);
        }
    }
    abl_str_stack_del(&dict->key_pool);
    abl_free(dict->buckets);
    abl_free(dict->sizes);
    abl_free(dict->capacities);
}

ABL_API void abl_dict_display(struct abl_dict* dict) {
    for(int i = 0; i < dict->numbuckets; ++ i) {
        if (dict->sizes[i]) {
            for(int j = 0; j < dict->sizes[i]; ++ j) {
                printf("%s => %p\n", dict->buckets[i][j].key, dict->buckets[i][j].value);
            }
        }
    }
}
