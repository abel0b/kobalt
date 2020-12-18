#ifndef KLMODGRAPH__H
#define KLMODGRAPH__H

#include "klbase/vec.h"
#include "klbase/dict.h"
#include "kobalt/ast.h"
#include "kobalt/astinfo.h"
#include "kobalt/mod.h"
#include "klbase/objpool.h"

struct kl_modgraph {
    struct kl_objpool mod_pool;
    struct kl_dict mods;
};

void kl_modgraph_new(struct kl_modgraph* modgraph);

struct kl_mod* kl_modgraph_add(struct kl_modgraph* modgraph, struct kl_str* modid, struct kl_ast* ast);

struct kl_mod* kl_modgraph_get(struct kl_modgraph* modgraph, struct kl_str* modid);

struct kl_mod* kl_modgraph_try_get(struct kl_modgraph* modgraph, struct kl_str* modid);

void kl_modgraph_depend(struct kl_modgraph* modgraph, struct kl_str* modid, struct kl_str* depid);

void kl_modgraph_del(struct kl_modgraph* modgraph);

struct kl_symbol* kl_modgraph_resolve(struct kl_modgraph* modgraph, struct kl_str* modid, int nid, struct kl_str* name);

struct kl_symbol* kl_modgraph_try_resolve(struct kl_modgraph* modgraph, struct kl_str* modid, int nid, struct kl_str* name);

struct kl_symbol* kl_modgraph_define(struct kl_modgraph* modgraph, struct kl_str* modid, int nid, struct kl_str* name);

struct kl_symbol* kl_modgraph_try_resolve_local(struct kl_modgraph* modgraph, struct kl_str* modid, int nid, struct kl_str* name);

#endif
