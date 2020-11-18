#include "kobalt/astinfo.h"
#include <assert.h>

kbvec_impl(struct kbscope*, scope)

static void gen_kbscope_del(void* data) {
    struct kbscope* scope = (struct kbscope*)data;
    kbdict_del(&scope->data);
}

static void gen_kbtype_del(void* data) {
    struct kbtype* type = (struct kbtype*)data;
    kbtype_del(type);
}

static void gen_kbsymbol_del(void* data) {
    struct kbsymbol* symbol = (struct kbsymbol*)data;
    kbsymbol_del(symbol);
}

void kbastinfo_new(struct kbastinfo* astinfo) {
    kbobjpool_new(&astinfo->type_pool, sizeof(struct kbtype), gen_kbtype_del);
    kbobjpool_new(&astinfo->scope_pool, sizeof(struct kbscope), gen_kbscope_del);
    kbobjpool_new(&astinfo->symbol_pool, sizeof(struct kbsymbol), gen_kbsymbol_del);
    kbvec_scope_new(&astinfo->scopes);
    kbvec_type_new(&astinfo->types);
}

void kbastinfo_del(struct kbastinfo* astinfo) {
    kbobjpool_del(&astinfo->type_pool);
    kbobjpool_del(&astinfo->scope_pool);
    kbobjpool_del(&astinfo->symbol_pool);
    kbvec_scope_del(&astinfo->scopes);
    kbvec_type_del(&astinfo->types);
}

struct kbscope* kbastinfo_alloc_scope(struct kbastinfo* astinfo, struct kbscope* pscope) {
    struct kbscope* scope = (struct kbscope*)kbobjpool_alloc(&astinfo->scope_pool);
    kbdict_new(&scope->data);
    scope->parent = pscope;
    return scope;
}

struct kbsymbol* kbscope_resolve(struct kbastinfo* astinfo, char* name, int nid) {
    struct kbsymbol* symbol = kbscope_try_resolve(astinfo, name, nid);
    if (!symbol) {
        kbelog("undefined symbol '%s'", name);
        exit(1);
    }
    return symbol;
}

struct kbsymbol* kbscope_try_resolve(struct kbastinfo* astinfo, char* name, int nid) {
    struct kbscope* scope = astinfo->scopes.data[nid];
    while(scope != NULL) {
        void* symbol = kbdict_get(&scope->data, name);
        if (symbol) {
            return (struct kbsymbol*)symbol;
        }
        scope = scope->parent;
    }
    return NULL;
}

struct kbsymbol* kbscope_define(struct kbastinfo* astinfo, char* name, int nid) {
    struct kbsymbol* symbol = (struct kbsymbol*) kbobjpool_alloc(&astinfo->symbol_pool);
    struct kbscope* scope = astinfo->scopes.data[nid];
    if (!scope) {
        kbelog("node '%d' has no defined scope", nid);
        exit(1);
    }
    kbdict_set(&scope->data, name, (void*) symbol);
    return symbol;
}
