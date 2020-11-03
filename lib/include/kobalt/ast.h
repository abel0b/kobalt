#ifndef KBAST_H
#define KBAST_H

#include "kobalt/kobalt.h"
#include "kobalt/syntax.h"
#include "kobalt/dict.h"
#include "kobalt/objpool.h"
#include <stdio.h>

struct kbast {
    struct kbnode* nodes;
    int capacity;
    int numnodes;
};

enum kbastorder {
    PreOrder = 1,
    PostOrder = 2,
    MixOrder = 3,
};

struct kbastvisitor {
    struct kbast* ast;
    enum kbastorder order;
    enum kbastorder curop;
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

void kbast_display(FILE* out, struct kbast* ast);

void kbastvisitor_new(struct kbast* ast, void* ctx, int (*visit)(struct kbastvisitor *), struct kbastvisitor* astvisitor, enum kbastorder order);

int kbastvisitor_step(struct kbastvisitor* astvisitor);

void kbastvisitor_run(struct kbastvisitor* astvisitor);

void kbastvisitor_del(struct kbastvisitor* astvisitor);

struct kbdict* kbscope_get(struct kbast* ast, int nid);

int kbscope_isdef(struct kbast* ast, int nid, char* name);

#endif
