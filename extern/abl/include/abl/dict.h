#ifndef ABLDICT__H
#define ABLDICT__H

#include "abl/strstack.h"
#include "abl/defs.h"
#include <stdint.h>

struct abl_dictentry {
    char* key;
    void* value;
};

struct abl_dict {
    struct abl_str_stack key_pool;
    int numbuckets;
    struct abl_dictentry** buckets;
    int* sizes;
    int* capacities;
};

ABL_API void abl_dict_new(struct abl_dict* dict);
ABL_API void abl_dict_set(struct abl_dict* dict, char* key, void* value);
ABL_API void* abl_dict_get(struct abl_dict* dict, char* key);
ABL_API void abl_dict_display(struct abl_dict* dict);
ABL_API void abl_dict_del(struct abl_dict* dict);

#endif
