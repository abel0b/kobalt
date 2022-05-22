#ifndef KLMODGRAPH__H
#define KLMODGRAPH__H

#include "abl/vec.h"
#include "abl/dict.h"
#include "kobalt/ast.h"
#include "kobalt/astinfo.h"
#include "kobalt/mod.h"
#include "abl/objpool.h"

struct kl_modgraph {
    struct abl_objpool mod_pool;
    struct abl_dict mods;
};

void kl_modgraph_new(struct kl_modgraph* modgraph);

struct kl_mod* kl_modgraph_add(struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_ast* ast);

struct kl_mod* kl_modgraph_get(struct kl_modgraph* modgraph, struct abl_str* modid);

struct kl_mod* kl_modgraph_try_get(struct kl_modgraph* modgraph, struct abl_str* modid);

void kl_modgraph_depend(struct kl_modgraph* modgraph, struct abl_str* modid, struct abl_str* depid);

void kl_modgraph_del(struct kl_modgraph* modgraph);

struct kl_symbol* kl_modgraph_resolve(struct kl_modgraph* modgraph, struct abl_str* modid, int nid, struct abl_str* name);

struct kl_symbol* kl_modgraph_try_resolve(struct kl_modgraph* modgraph, struct abl_str* modid, int nid, struct abl_str* name);

struct kl_symbol* kl_modgraph_define(struct kl_modgraph* modgraph, struct abl_str* modid, int nid, struct abl_str* name);

struct kl_symbol* kl_modgraph_try_resolve_local(struct kl_modgraph* modgraph, struct abl_str* modid, int nid, struct abl_str* name);

#endif
