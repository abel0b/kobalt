#include "kobalt/modgraph.h"

#define decl_del_void(DEL, TYPE) void DEL##_void(void* data) {\
    TYPE* t_data = (TYPE*) data;\
    DEL(t_data);\
}

decl_del_void(kbmod_del, struct kbmod)

void kbmodgraph_new(struct kbmodgraph* modgraph) {
    kbobjpool_new(&modgraph->mod_pool, sizeof(struct kbmod), kbmod_del_void);
    kbdict_new(&modgraph->mods);
}

struct kbmod* kbmodgraph_add(struct kbmodgraph* modgraph, struct kbstr* modid, struct kbast* ast) {
    struct kbmod* mod = (struct kbmod*) kbobjpool_alloc(&modgraph->mod_pool);
    kbmod_new(mod, ast);
    kbdict_set(&modgraph->mods, modid->data, (void*) mod);
    return mod;
}

struct kbmod* kbmodgraph_get(struct kbmodgraph* modgraph, struct kbstr* modid) {
    struct kbmod* mod = kbmodgraph_try_get(modgraph, modid);
    if (!mod) {
        kbelog("undefined module '%s'", modid->data);
        exit(1);
    }
    return mod;
}

struct kbmod* kbmodgraph_try_get(struct kbmodgraph* modgraph, struct kbstr* modid) {
    struct kbmod* mod = (struct kbmod*) kbdict_get(&modgraph->mods, modid->data);
    return mod;
}

void kbmodgraph_depend(struct kbmodgraph* modgraph, struct kbstr* modid, struct kbstr* depid) {
    struct kbmod* mod = (struct kbmod*) kbdict_get(&modgraph->mods, modid->data);
    kbvec_str_push(&mod->deps, *depid);
}

void kbmodgraph_del(struct kbmodgraph* modgraph) {
    kbobjpool_del(&modgraph->mod_pool);
    kbdict_del(&modgraph->mods);
}

struct kbsymbol* kbmodgraph_resolve(struct kbmodgraph* modgraph, struct kbstr* modid, int nid, struct kbstr* name) {
    struct kbsymbol* symbol = kbmodgraph_try_resolve(modgraph, modid, nid, name);
    if (!symbol) {
        kbelog("undefined symbol '%s'", name->data);
        exit(1);
    }
    return symbol;
}

static struct kbsymbol* kbmodgraph_try_resolve_rec(struct kbmodgraph* modgraph, struct kbstr* modid, int nid, struct kbstr* name, int maxdepth) {
    // TODO: cycle detection algorithm
    if (maxdepth == 0) {
        kbelog("recursion limit reached, there may be a module cycle");
        exit(1);
    }
    
    struct kbmod* mod = (struct kbmod*) kbdict_get(&modgraph->mods, modid->data);

    struct kbscope* scope = mod->astinfo.scopes.data[nid];
    while(scope != NULL) {
        struct kbsymbol* symbol = (struct kbsymbol*) kbdict_get(&scope->data, name->data);
        if (symbol) {
            return symbol;
        }
        scope = scope->parent;
    }

    for(int i = 0; i < mod->deps.size; ++ i) {
        struct kbsymbol* symbol = kbmodgraph_try_resolve_rec(modgraph, &mod->deps.data[i], 0, name, maxdepth - 1);
        if (symbol) {
            return symbol;
        }
    }

    return NULL;
}

struct kbsymbol* kbmodgraph_try_resolve(struct kbmodgraph* modgraph, struct kbstr* modid, int nid, struct kbstr* name) {
    return kbmodgraph_try_resolve_rec(modgraph, modid, nid, name, 8);
}

struct kbsymbol* kbmodgraph_define(struct kbmodgraph* modgraph, struct kbstr* modid, int nid, struct kbstr* name) {
    struct kbmod* mod = (struct kbmod*) kbdict_get(&modgraph->mods, modid->data);
    assert(mod);

    struct kbsymbol* symbol = (struct kbsymbol*) kbobjpool_alloc(&mod->astinfo.symbol_pool);
    struct kbscope* scope = mod->astinfo.scopes.data[nid];
    if (!scope) {
        kbelog("node '%d' has no defined scope", nid);
        exit(1);
    }
    kbdict_set(&scope->data, name->data, (void*) symbol);
    return symbol;
}
