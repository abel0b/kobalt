#include "kobalt/astinfo.h"
#include "kobalt/type.h"
#include <assert.h>

static void gen_kbscope_del(void* data) {
    struct kbscope* scope = (struct kbscope*)data;
    kbdict_del(&scope->data);
}

static void gen_kbtype_del(void* data) {
    struct kbtype* type = (struct kbtype*)data;
    kbtype_del(type);
}

void kbastinfo_new(struct kbastinfo* astinfo) {
    kbobjpool_new(&astinfo->type_pool, sizeof(struct kbtype), gen_kbtype_del);
    kbobjpool_new(&astinfo->scope_pool, sizeof(struct kbscope), gen_kbscope_del);
    kbdict_new(&astinfo->scopes);
}

void kbastinfo_del(struct kbastinfo* astinfo) {
    kbobjpool_del(&astinfo->type_pool);
    kbobjpool_del(&astinfo->scope_pool);
    kbdict_del(&astinfo->scopes);
}

struct kbscope* kbastinfo_alloc_scope(struct kbastinfo* astinfo, struct kbscope* pscope) {
    struct kbscope* scope = (struct kbscope*)kbobjpool_alloc(&astinfo->scope_pool);
    kbdict_new(&scope->data);
    scope->parent = pscope;
    return scope;
}

void* kbscope_resolve(struct kbastinfo* astinfo, uintptr_t name, int nid) {
    struct kbscope* scope = (struct kbscope*)kbdict_get(&astinfo->scopes, (uintptr_t)nid);
    while(scope != NULL) {
        void* symbol = kbdict_get(&scope->data, (uintptr_t)name);
        if (symbol) return symbol;
        scope = scope->parent;
    }
    return NULL;
}

void kbscope_define(struct kbscope* scope, uintptr_t name, void* data) {
    kbdict_set(&scope->data, name, data);
    assert(kbdict_get(&scope->data, name));
}
