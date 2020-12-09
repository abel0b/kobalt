#ifndef KLAST_H
#define KLAST_H

#include "kobalt/kobalt.h"
#include "kobalt/syntax.h"
#include "klbase/dict.h"
#include "klbase/objpool.h"
#include "klbase/vec.h"
#include <stdio.h>

kl_vec_decl(struct kl_node, node)

struct kl_ast {
    struct kl_vec_node nodes;
};

enum kl_astorder {
    PreOrder = 1,
    PostOrder = 2,
    MixOrder = 3,
};

struct kl_astvisit {
    struct kl_ast* ast;
    enum kl_astorder order;
    enum kl_astorder curop;
    struct {int nid; int depth;} * stack;
    struct {int nid; int depth;} cur;
    void* ctx;
    int stacksize;
    int stackcap;
    int (*visit)(struct kl_astvisit* astvisit);
};

void kl_ast_new(struct kl_ast* ast);

int kl_ast_add(struct kl_ast* ast, enum kl_node_kind kind, int parent, struct kl_loc loc);

void kl_ast_del(struct kl_ast* ast);

void kl_astvisit_new(struct kl_ast* ast, void* ctx, int (*visit)(struct kl_astvisit *), struct kl_astvisit* astvisit, enum kl_astorder order);

int kl_astvisit_step(struct kl_astvisit* astvisit);

void kl_astvisit_run(struct kl_astvisit* astvisit);

void kl_astvisit_del(struct kl_astvisit* astvisit);

struct kl_dict* kl_scope_get(struct kl_ast* ast, int nid);

int kl_scope_isdef(struct kl_ast* ast, int nid, char* name);

#endif
