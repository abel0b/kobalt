#include "kobalt/ast.h"
#include "kobalt/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static int INITNUMNODES = 32;

void kbastvisitor_new(struct kbast* ast, void* ctx, int (*visit)(struct kbastvisitor *), struct kbastvisitor* astvisitor) {
    astvisitor->ast = ast;
    astvisitor->ctx = ctx;
    astvisitor->stackcap = 8;
    astvisitor->stacksize = 1;
    astvisitor->stack = kbmalloc(sizeof(astvisitor->stack[0]) * astvisitor->stackcap);
    astvisitor->stack[0].nid = 0;
    astvisitor->stack[0].depth = 0;
    astvisitor->visit = NULL;
    astvisitor->visit = visit;
}

static void pop(struct kbastvisitor * astvisitor) {
#ifdef DEBUG
    assert(astvisitor->stacksize);
#endif
    --astvisitor->stacksize;
    astvisitor->cur.nid = astvisitor->stack[astvisitor->stacksize].nid;
    astvisitor->cur.depth = astvisitor->stack[astvisitor->stacksize].depth;
}

static void push(struct kbastvisitor* astvisitor, int nid) {
    if (nid == -1) return;
    if (astvisitor->stacksize == astvisitor->stackcap) {
        astvisitor->stackcap *= 2;
        astvisitor->stack = kbrealloc(astvisitor->stack,  sizeof(astvisitor->stack[0]) * astvisitor->stackcap * 2);
    }
    astvisitor->stack[astvisitor->stacksize].nid = nid;
    astvisitor->stack[astvisitor->stacksize].depth = astvisitor->cur.depth+1;
    ++ astvisitor->stacksize;
}

struct kbnode * kbast_getnode(struct kbast * ast, int nid) {
    return &ast->nodes[nid];
}

static void pushall(struct kbastvisitor* astvisitor, struct kbnode* node) {
    for(int ii=node->data.group.numitems-1; ii>=0; --ii) {
        push(astvisitor, node->data.group.items[ii]);
    }
}

static void indent(FILE* out, int level) {
    for (int ii=0; ii<level; ++ii) {
        fprintf(out, "    ");
    }
}

int kbastvisitor_step(struct kbastvisitor* astvisitor) {
    if (astvisitor->stacksize == 0) return 0;
    pop(astvisitor);
    struct kbnode * node = kbast_getnode(astvisitor->ast, astvisitor->cur.nid);
    switch(node->kind) {
        case NFile:
            pushall(astvisitor, node);
            break;
        case NFun:
            push(astvisitor, node->data.fun.funbody);
            push(astvisitor, node->data.fun.funparams);
            push(astvisitor, node->data.fun.id);
            break;
        case NFunParam:
            push(astvisitor, node->data.funparam.type);
            push(astvisitor, node->data.funparam.id);
            break;
        case NFunParams:
            pushall(astvisitor, node);
            break;
        case NFunBody:
            pushall(astvisitor, node);
            break;
        case NDecl:
            break;
        case NType:
            break;
        case NExpr:
            push(astvisitor, node->data.expr.expr);
            break;
        case NTerm:
            break;
        case NBinExpr:
            break;
        case NStrLit:
            break;
        case NIntLit:
            break;
        case NFloatLit:
            break;
        case NCharLit:
            break;
        case NCall:
            push(astvisitor, node->data.call.callparams);
            push(astvisitor, node->data.call.id);
            break;
        case NAssign:
            break;
        case NId:
            break;
        case NCallParams:
            pushall(astvisitor, node);
            break;
        case NCallParam:
            push(astvisitor, node->data.callparam.expr);
            break;
    }
    if(astvisitor->visit) astvisitor->visit(astvisitor);
    return 1;
}

void kbastvisitor_run(struct kbastvisitor* astvisitor) {
    while(kbastvisitor_step(astvisitor));
}

void kbastvisitor_del(struct kbastvisitor* astvisitor) {
    kbfree(astvisitor->stack);
}

static int display_aux(struct kbastvisitor* astvisitor) {
    struct kbnode * node = kbast_getnode(astvisitor->ast, astvisitor->cur.nid);
    FILE* out = (FILE*)astvisitor->ctx;
    indent(out, astvisitor->cur.depth);
    fprintf(out, "%s", kbnode_kind_str(node->kind));
    switch(node->kind) {
        case NFile:
            fprintf(out, "\n");
            break;
        case NFun:
            fprintf(out, "\n");
            break;
        case NFunParam:
            fprintf(out, "\n");
            break;
        case NFunParams:
            fprintf(out, "\n");
            break;
        case NFunBody:
            fprintf(out, "\n");
            break;
        case NDecl:
            fprintf(out, "\n");
            break;
        case NType:
            fprintf(out, "\n");
            break;
        case NExpr:
            fprintf(out, "\n");
            break;
        case NTerm:
            fprintf(out, "\n");
            break;
        case NBinExpr:
            fprintf(out, "\n");
            break;
        case NStrLit:
            fprintf(out, " \"%s\"\n", node->data.strlit.value);
            break;
        case NIntLit:
            fprintf(out, " %d\n", node->data.intlit.value);
            break;
        case NFloatLit:
            fprintf(out, " %f\n", node->data.floatlit.value);
            break;
        case NCharLit:
            fprintf(out, " \'%c\'\n", node->data.charlit.value);
            break;
        case NCall:
            fprintf(out, "\n");
            break;
        case NAssign:
            fprintf(out, "\n");
            break;
        case NId:
            fprintf(out, " \"%s\"\n", node->data.id.name);
            break;
        case NCallParams:
            fprintf(out, "\n");
            break;
        case NCallParam:
            fprintf(out, "\n");
            break;
    }
    return 1;
}

void kbast_display(FILE* out, struct kbast* ast) {
    struct kbastvisitor visdisp;
    kbastvisitor_new(ast, out, display_aux, &visdisp);
    kbastvisitor_run(&visdisp);
    kbastvisitor_del(&visdisp);
}

static int del_aux(struct kbastvisitor* astvisitor) {
    struct kbnode * node = kbast_getnode(astvisitor->ast, astvisitor->cur.nid);
    switch(node->kind) {
        case NCallParams:
        case NFunBody:
        case NFunParams:
        case NFile:
            if(node->data.group.numitems) kbfree(node->data.group.items);
            break;
        case NFun:
            break;
        case NFunParam:
            break;
        case NDecl:
            break;
        case NType:
            break;
        case NExpr:
            break;
        case NCall:
            break;
        case NTerm:
            break;
        case NBinExpr:
            break;
        case NStrLit:
            kbfree(node->data.strlit.value);
            break;
        case NIntLit:
            break;
        case NFloatLit:
            break;
        case NCharLit:
            break;
        case NAssign:
            break;
        case NId:
            kbfree(node->data.id.name);
            break;
        case NCallParam:
            break;
    }
    return 1;
}

void kbast_new(struct kbast* ast) {
    ast->capacity = 0;
    ast->numnodes = 0;
    ast->nodes = NULL;
}

int kbast_add(struct kbast* ast, enum kbnode_kind kind, int parent) {
    if (ast->numnodes == ast->capacity) {
        ast->capacity = (ast->capacity == 0)? INITNUMNODES : 2 * ast->capacity;
        ast->nodes = kbrealloc(ast->nodes, sizeof(ast->nodes[0]) * ast->capacity);
    }
    ast->nodes[ast->numnodes].kind = kind;
    ast->nodes[ast->numnodes].parent = parent;
    memset(&ast->nodes[ast->numnodes].data, 0, sizeof(ast->nodes[ast->numnodes].data));
    return ast->numnodes++;
}

void kbast_del(struct kbast * ast) {
    struct kbastvisitor visdel;
    kbastvisitor_new(ast, NULL, del_aux, &visdel);
    kbastvisitor_run(&visdel);
    kbastvisitor_del(&visdel);
    if(ast->capacity) kbfree(ast->nodes);
}
