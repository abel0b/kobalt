#ifndef KBMODGRAPH__H
#define KBMODGRAPH__H

#include "kobalt/vec.h"
#include "kobalt/dict.h"
#include "kobalt/ast.h"
#include "kobalt/astinfo.h"
#include "kobalt/mod.h"
#include "kobalt/objpool.h"

struct kbmodgraph {
    struct kbobjpool mod_pool;
    struct kbdict mods;
};

void kbmodgraph_new(struct kbmodgraph* modgraph);

struct kbmod* kbmodgraph_add(struct kbmodgraph* modgraph, struct kbstr* modid, struct kbast* ast);

struct kbmod* kbmodgraph_get(struct kbmodgraph* modgraph, struct kbstr* modid);

struct kbmod* kbmodgraph_try_get(struct kbmodgraph* modgraph, struct kbstr* modid);

void kbmodgraph_depend(struct kbmodgraph* modgraph, struct kbstr* modid, struct kbstr* depid);

void kbmodgraph_del(struct kbmodgraph* modgraph);

struct kbsymbol* kbmodgraph_resolve(struct kbmodgraph* modgraph, struct kbstr* modid, int nid, struct kbstr* name);

struct kbsymbol* kbmodgraph_try_resolve(struct kbmodgraph* modgraph, struct kbstr* modid, int nid, struct kbstr* name);

struct kbsymbol* kbmodgraph_define(struct kbmodgraph* modgraph, struct kbstr* modid, int nid, struct kbstr* name);

#endif
