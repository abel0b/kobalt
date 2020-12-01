#ifndef KBAST_H
#define KBAST_H

#include "kobalt/kobalt.h"
#include "kobalt/syntax.h"
#include "kobalt/dict.h"
#include "kobalt/objpool.h"
#include "kobalt/vec.h"
#include <stdio.h>

kbvec_decl(struct kbnode, node)

struct kbast {
    struct kbvec_node nodes;
};

enum kbastorder {
    PreOrder = 1,
    PostOrder = 2,
    MixOrder = 3,
};

struct kbastvisit {
    struct kbast* ast;
    enum kbastorder order;
    enum kbastorder curop;
    struct {int nid; int depth;} * stack;
    struct {int nid; int depth;} cur;
    void* ctx;
    int stacksize;
    int stackcap;
    int (*visit)(struct kbastvisit* astvisit);
};

void kbast_new(struct kbast* ast);

int kbast_add(struct kbast* ast, enum kbnode_kind kind, int parent, struct kbloc loc);

void kbast_del(struct kbast* ast);

void kbastvisit_new(struct kbast* ast, void* ctx, int (*visit)(struct kbastvisit *), struct kbastvisit* astvisit, enum kbastorder order);

int kbastvisit_step(struct kbastvisit* astvisit);

void kbastvisit_run(struct kbastvisit* astvisit);

void kbastvisit_del(struct kbastvisit* astvisit);

struct kbdict* kbscope_get(struct kbast* ast, int nid);

int kbscope_isdef(struct kbast* ast, int nid, char* name);

#endif
