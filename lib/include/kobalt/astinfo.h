#ifndef KBASTINFO__H
#define KBASTINFO__H

#include "kobalt/dict.h"
#include "kobalt/vec.h"
#include "kobalt/objpool.h"
#include "kobalt/type.h"
#include "kobalt/symbol.h"
#include <stdint.h>

struct kbscope {
    struct kbdict data;
    struct kbscope* parent; 
};

kbvec_decl(struct kbscope*, scope)

struct kbastinfo {
    struct kbobjpool type_pool;
    struct kbobjpool scope_pool;
    struct kbobjpool symbol_pool;
    struct kbvec_scope scopes;
    struct kbvec_type types;
};

struct kbscope* kbastinfo_alloc_scope(struct kbastinfo* astinfo, struct kbscope* pscope);

void kbastinfo_new(struct kbastinfo* astinfo);

void kbastinfo_del(struct kbastinfo* astinfo);

struct kbsymbol* kbscope_resolve(struct kbastinfo* astinfo, char* name, int nid);

struct kbsymbol* kbscope_try_resolve(struct kbastinfo* astinfo, char* name, int nid);

struct kbsymbol* kbscope_define(struct kbastinfo* astinfo, char* name, int nid);

#endif
