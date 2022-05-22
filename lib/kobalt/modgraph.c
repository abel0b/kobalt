#include "kobalt/modgraph.h"

#define decl_del_void(DEL, TYPE) void DEL##_void(void* data) {\
    TYPE* t_data = (TYPE*) data;\
    DEL(t_data);\
}

decl_del_void(kl_mod_del, struct kl_mod)

void kl_modgraph_new(struct kl_modgraph* modgraph) {
    abl_objpool_new(&modgraph->mod_pool, sizeof(struct kl_mod), kl_mod_del_void);
    abl_dict_new(&modgraph->mods);
}

struct kl_mod* kl_modgraph_add(struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_ast* ast) {
    struct kl_mod* mod = (struct kl_mod*) abl_objpool_alloc(&modgraph->mod_pool);
    kl_mod_new(mod, ast);
    abl_dict_set(&modgraph->mods, modid->data, (void*) mod);
    return mod;
}

struct kl_mod* kl_modgraph_get(struct kl_modgraph* modgraph, struct abl_str* modid) {
    struct kl_mod* mod = kl_modgraph_try_get(modgraph, modid);
    if (!mod) {
        abl_elog("undefined module '%s'", modid->data);
        exit(1);
    }
    return mod;
}

struct kl_mod* kl_modgraph_try_get(struct kl_modgraph* modgraph, struct abl_str* modid) {
    struct kl_mod* mod = (struct kl_mod*) abl_dict_get(&modgraph->mods, modid->data);
    return mod;
}

void kl_modgraph_depend(struct kl_modgraph* modgraph, struct abl_str* modid, struct abl_str* depid) {
    struct kl_mod* mod = (struct kl_mod*) abl_dict_get(&modgraph->mods, modid->data);
    abl_vec_str_push(&mod->deps, *depid);
}

void kl_modgraph_del(struct kl_modgraph* modgraph) {
    abl_objpool_del(&modgraph->mod_pool);
    abl_dict_del(&modgraph->mods);
}

struct kl_symbol* kl_modgraph_resolve(struct kl_modgraph* modgraph, struct abl_str* modid, int nid, struct abl_str* name) {
    struct kl_symbol* symbol = kl_modgraph_try_resolve(modgraph, modid, nid, name);
    if (!symbol) {
        abl_elog("undefined symbol '%s'", name->data);
        exit(1);
    }
    return symbol;
}

static struct kl_symbol* kl_modgraph_try_resolve_rec(struct kl_modgraph* modgraph, struct abl_str* modid, int nid, struct abl_str* name, int maxdepth) {
    // TODO: cycle detection algorithm
    if (maxdepth == 0) {
        abl_elog("recursion limit reached, there may be a module cycle");
        exit(1);
    }
    
    struct kl_mod* mod = (struct kl_mod*) abl_dict_get(&modgraph->mods, modid->data);

    struct kl_scope* scope = mod->astinfo.scopes.data[nid];
    while(scope != NULL) {
        struct kl_symbol* symbol = (struct kl_symbol*) abl_dict_get(&scope->data, name->data);
        if (symbol) {
            return symbol;
        }
        scope = scope->parent;
    }

    for(int i = 0; i < mod->deps.size; ++ i) {
        struct kl_symbol* symbol = kl_modgraph_try_resolve_rec(modgraph, &mod->deps.data[i], 0, name, maxdepth - 1);
        if (symbol) {
            return symbol;
        }
    }

    return NULL;
}

struct kl_symbol* kl_modgraph_try_resolve_local(struct kl_modgraph* modgraph, struct abl_str* modid, int nid, struct abl_str* name) {
    struct kl_mod* mod = (struct kl_mod*) abl_dict_get(&modgraph->mods, modid->data);
    struct kl_scope* scope = mod->astinfo.scopes.data[nid];
    struct kl_symbol* symbol = (struct kl_symbol*) abl_dict_get(&scope->data, name->data);
    return symbol;
}

struct kl_symbol* kl_modgraph_try_resolve(struct kl_modgraph* modgraph, struct abl_str* modid, int nid, struct abl_str* name) {
    return kl_modgraph_try_resolve_rec(modgraph, modid, nid, name, 8);
}

struct kl_symbol* kl_modgraph_define(struct kl_modgraph* modgraph, struct abl_str* modid, int nid, struct abl_str* name) {
    struct kl_mod* mod = (struct kl_mod*) abl_dict_get(&modgraph->mods, modid->data);
    assert(mod);

    struct kl_symbol* symbol = (struct kl_symbol*) abl_objpool_alloc(&mod->astinfo.symbol_pool);
    struct kl_scope* scope = mod->astinfo.scopes.data[nid];
    if (!scope) {
        abl_elog("node '%d' has no defined scope", nid);
        exit(1);
    }
    abl_dict_set(&scope->data, name->data, (void*) symbol);
    return symbol;
}
