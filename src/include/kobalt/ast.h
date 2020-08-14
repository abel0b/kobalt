#ifndef KBAST_H
#define KBAST_H

#include "kobalt/kobalt.h"
#include "kobalt/syntax.h"

struct kbast {
    struct kbnode* nodes;
    int capacity;
    int numnodes;
};

struct kbastvisitor {
    struct kbast* ast;
    struct {int nid; int depth;} * stack;
    struct {int nid; int depth;} cur;
    void* ctx;
    int stacksize;
    int stackcap;
    int (*visit)(struct kbastvisitor* astvisitor);
};

void kbast_new(struct kbast* ast);

int kbast_add(struct kbast* ast, enum kbnode_kind kind, int parent);

void kbast_del(struct kbast* ast);

struct kbnode* kbast_getnode(struct kbast* ast, int nid);

void kbast_display(struct kbast* ast);

void kbastvisitor_new(struct kbast* ast, void* ctx, int (*visit)(struct kbastvisitor *), struct kbastvisitor* astvisitor);

int kbastvisitor_step(struct kbastvisitor* astvisitor);

void kbastvisitor_run(struct kbastvisitor* astvisitor);

void kbastvisitor_del(struct kbastvisitor* astvisitor);

#endif
