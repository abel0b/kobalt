#ifndef KBASTINFO__H
#define KBASTINFO__H

#include "kobalt/dict.h"
#include "kobalt/vec.h"
#include "kobalt/objpool.h"
#include <stdint.h>

struct kbscope {
    struct kbdict data;
    struct kbscope* parent; 
};

struct kbastinfo {
    struct kbobjpool type_pool;
    struct kbobjpool scope_pool;
    struct kbdict scopes;
};

struct kbscope* kbastinfo_alloc_scope(struct kbastinfo* astinfo, struct kbscope* pscope);

void kbastinfo_new(struct kbastinfo* astinfo);

void kbastinfo_del(struct kbastinfo* astinfo);

void* kbscope_resolve(struct kbastinfo* astinfo, uintptr_t name, int nid);

void kbscope_define(struct kbscope* scope, uintptr_t name, void* data);

#endif
