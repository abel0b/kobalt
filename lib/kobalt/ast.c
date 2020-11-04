#include "kobalt/ast.h"
#include "kobalt/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "kobalt/type.h"
#include "kobalt/log.h"

static int INITNUMNODES = 32;

void kbastvisitor_new(struct kbast* ast, void* ctx, int (*visit)(struct kbastvisitor *), struct kbastvisitor* astvisitor, enum kbastorder order) {
    astvisitor->order = order;
    astvisitor->ast = ast;
    astvisitor->ctx = ctx;
    astvisitor->visit = NULL;
    astvisitor->visit = visit;
}

static void indent(FILE* out, int level) {
    for (int ii=0; ii<level; ++ii) {
        fprintf(out, "    ");
    }
}

int kbastvisitor_rec(struct kbastvisitor* astvisitor, int nid, int depth) {
    if (nid == -1) return 0;
    struct kbnode* node = &astvisitor->ast->nodes[nid];
    astvisitor->cur.nid = nid;
    astvisitor->cur.depth = depth;

    if(astvisitor->order & PreOrder) {
        astvisitor->curop = PreOrder;
        astvisitor->visit(astvisitor);
    }
    
    switch(node->kind) {
        case NFile:
        case NSeq:
        case NFunParams:
        case NIfElse:
        case NCallParams:
            for(int i = 0; i < node->data.group.numitems; ++i) {
                kbastvisitor_rec(astvisitor, node->data.group.items[i], depth + 1);
            }
            break;
        case NFun:
            kbastvisitor_rec(astvisitor, node->data.fun.id, depth + 1);
            kbastvisitor_rec(astvisitor, node->data.fun.funparams, depth + 1);
            kbastvisitor_rec(astvisitor, node->data.fun.rettype, depth + 1);
            kbastvisitor_rec(astvisitor, node->data.fun.funbody, depth + 1);
            break;
        case NFunParam:
            kbastvisitor_rec(astvisitor, node->data.funparam.id, depth + 1);
            kbastvisitor_rec(astvisitor, node->data.funparam.type, depth + 1);
            break;
        case NCall:
            kbastvisitor_rec(astvisitor, node->data.call.id, depth + 1);
            kbastvisitor_rec(astvisitor, node->data.call.callparams, depth + 1);
            break;
        case NAssign:
            break;
        case NId:
            break;
        case NCallParam:
            kbastvisitor_rec(astvisitor, node->data.callparam.expr, depth + 1);
            break;
        case NIfBranch:
        case NElifBranch:
            kbastvisitor_rec(astvisitor, node->data.ifbranch.cond, depth + 1);
            kbastvisitor_rec(astvisitor, node->data.ifbranch.conseq, depth + 1);
            break;
        case NElseBranch:
            kbastvisitor_rec(astvisitor, node->data.ifbranch.conseq, depth + 1);
        default:
            break;
    }

    astvisitor->cur.nid = nid;
    astvisitor->cur.depth = depth;
    if(astvisitor->order & PostOrder) {
        astvisitor->curop = PostOrder;
        astvisitor->visit(astvisitor);
    }
    return 1;
}

void kbastvisitor_run(struct kbastvisitor* astvisitor) {
    kbastvisitor_rec(astvisitor, 0, 0);
}

void kbastvisitor_del(struct kbastvisitor* astvisitor) {
    unused(astvisitor);
}

static int display_aux(struct kbastvisitor* astvisitor) {
    struct kbnode* node = &astvisitor->ast->nodes[astvisitor->cur.nid];
    FILE* out = (FILE*)astvisitor->ctx;
    indent(out, astvisitor->cur.depth);
    fprintf(out, "%s", kbnode_kind_str(node->kind));
    switch(node->kind) {
        case NType:
            fprintf(out, " \"%s\"\n", node->data.type.name);
            break;
        case NStrLit:
            fprintf(out, " \"%s\"\n", node->data.strlit.value);
            break;
        case NIntLit:
            fprintf(out, " %s\n", node->data.intlit.value);
            break;
        case NFloatLit:
            fprintf(out, " %s\n", node->data.floatlit.value);
            break;
        case NCharLit:
            fprintf(out, " \'%s\'\n", node->data.charlit.value);
            break;
        case NId:
            fprintf(out, " \"%s\"\n", node->data.id.name);
            break;
        case NSym:
            fprintf(out, " \"%s\"\n", kbtoken_string(node->data.sym.kind));
            break;
        default:
            fprintf(out, "\n");
            break;
    }
    return 1;
}

void kbast_display(FILE* out, struct kbast* ast) {
    struct kbastvisitor visdisp;
    kbastvisitor_new(ast, out, display_aux, &visdisp, PreOrder);
    kbastvisitor_run(&visdisp);
    kbastvisitor_del(&visdisp);
}

static int del_aux(struct kbastvisitor* astvisitor) {
    struct kbnode* node = &astvisitor->ast->nodes[astvisitor->cur.nid];
    switch(node->kind) {
        case NCallParams:
        case NSeq:
        case NFunParams:
        case NFile:
        case NIfElse:
            if(node->data.group.numitems) {
                kbfree(node->data.group.items);
            }
            break;
        case NType:
            kbfree(node->data.type.name);
            break;
        case NCall:
            break;
        case NStrLit:
            kbfree(node->data.strlit.value);
            break;
        case NIntLit:
            kbfree(node->data.intlit.value);
            break;
        case NFloatLit:
            kbfree(node->data.floatlit.value);
            break;
        case NCharLit:
            break;
        case NAssign:
            break;
        case NId:
            kbfree(node->data.id.name);
            break;
        default:
            break;
    }
    return 1;
}

static void kbtype_del_aux(void* data) {
    struct kbtype* type = (struct kbtype*) data;
    kbtype_del(type);
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

void kbast_del(struct kbast* ast) {
    struct kbastvisitor visdel;
    kbastvisitor_new(ast, NULL, del_aux, &visdel, PostOrder);
    kbastvisitor_run(&visdel);
    kbastvisitor_del(&visdel);
    if(ast->capacity) {
        kbfree(ast->nodes);
    }
}
