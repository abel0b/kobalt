#include "kobalt/ast.h"
#include "abl/mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "kobalt/type.h"
#include "abl/log.h"
#include "abl/vec.h"

abl_vec_impl(struct kl_node, node)

void kl_ast_new(struct kl_ast* ast) {
    abl_vec_node_new(&ast->nodes);
}

void kl_astvisit_new(struct kl_ast* ast, void* ctx, int (*visit)(struct kl_astvisit *), struct kl_astvisit* astvisit, enum kl_astorder order) {
    astvisit->order = order;
    astvisit->ast = ast;
    astvisit->ctx = ctx;
    astvisit->visit = NULL;
    astvisit->visit = visit;
}

static void indent(FILE* out, int level) {
    for (int ii=0; ii<level; ++ii) {
        fprintf(out, "  ");
    }
}

int kl_astvisit_rec(struct kl_astvisit* astvisit, int nid, int depth) {
    if (nid == -1) {
        return 0;
    }
    struct kl_node* node = &astvisit->ast->nodes.data[nid];
    astvisit->cur.nid = nid;
    astvisit->cur.depth = depth;
    // abl_ilog("VISIT %s", kl_node_kind_str(node->kind));

    if(astvisit->order & PreOrder) {
        astvisit->curop = PreOrder;
        astvisit->visit(astvisit);
    }
    
    switch(node->kind) {
        case NProgram:
        case NSeq:
        case NFunParams:
        case NIfElse:
        case NCallParams:
            for(int i = 0; i < node->data.group.numitems; ++i) {
                kl_astvisit_rec(astvisit, node->data.group.items[i], depth + 1);
            }
            break;
        case NForLoop:
            kl_astvisit_rec(astvisit, node->data.forloop.id, depth + 1);
            kl_astvisit_rec(astvisit, node->data.forloop.start, depth + 1);
            kl_astvisit_rec(astvisit, node->data.forloop.end, depth + 1);
            kl_astvisit_rec(astvisit, node->data.forloop.expr, depth + 1);

            break;
        case NFun:
            kl_astvisit_rec(astvisit, node->data.fun.id, depth + 1);
            kl_astvisit_rec(astvisit, node->data.fun.funparams, depth + 1);
            kl_astvisit_rec(astvisit, node->data.fun.rettype, depth + 1);
            kl_astvisit_rec(astvisit, node->data.fun.body, depth + 1);
            break;
        case NFunParam:
            kl_astvisit_rec(astvisit, node->data.funparam.id, depth + 1);
            kl_astvisit_rec(astvisit, node->data.funparam.type, depth + 1);
            break;
        case NCall:
            kl_astvisit_rec(astvisit, node->data.call.id, depth + 1);
            kl_astvisit_rec(astvisit, node->data.call.callparams, depth + 1);
            break;
        case NAssign:
            break;
        case NId:
            break;
        case NCallParam:
            kl_astvisit_rec(astvisit, node->data.callparam.expr, depth + 1);
            break;
        case NVal:
            kl_astvisit_rec(astvisit, node->data.val.id, depth + 1);
            kl_astvisit_rec(astvisit, node->data.val.expr, depth + 1);
            break;
        case NIfBranch:
        case NElifBranch:
            kl_astvisit_rec(astvisit, node->data.ifbranch.cond, depth + 1);
            kl_astvisit_rec(astvisit, node->data.ifbranch.conseq, depth + 1);
            break;
        case NElseBranch:
            kl_astvisit_rec(astvisit, node->data.ifbranch.conseq, depth + 1);
        default:
            break;
    }

    astvisit->cur.nid = nid;
    astvisit->cur.depth = depth;
    if(astvisit->order & PostOrder) {
        astvisit->curop = PostOrder;
        astvisit->visit(astvisit);
    }
    return 1;
}

void kl_astvisit_run(struct kl_astvisit* astvisit) {
    kl_astvisit_rec(astvisit, 0, 0);
}

void kl_astvisit_del(struct kl_astvisit* astvisit) {
    unused(astvisit);
}

static int del_aux(struct kl_astvisit* astvisit) {
    struct kl_node* node = &astvisit->ast->nodes.data[astvisit->cur.nid];
    switch(node->kind) {
        case NCallParams:
        case NSeq:
        case NFunParams:
        case NProgram:
        case NIfElse:
            if(node->data.group.numitems) {
                abl_free(node->data.group.items);
            }
            break;
        case NType:
            abl_free(node->data.type.name);
            break;
        case NCall:
            break;
        case NStrLit:
            abl_free(node->data.strlit.value);
            break;
        case NIntLit:
            abl_free(node->data.intlit.value);
            break;
        case NFloatLit:
            abl_free(node->data.floatlit.value);
            break;
        case NCharLit:
            abl_free(node->data.floatlit.value);
            break;
        case NAssign:
            break;
        case NId:
            abl_free(node->data.id.name);
            break;
        case NImport:
            abl_free(node->data.import.path);
            break;
        default:
            break;
    }
    return 1;
}

int kl_ast_add(struct kl_ast* ast, enum kl_node_kind kind, int parent, struct kl_loc loc) {
    struct kl_node node = {
        .kind = kind,
        .parent = parent,
        .loc = loc,
    };
    memset(&node.data, 0, sizeof(node.data));
    abl_vec_node_push(&ast->nodes, node);
    return ast->nodes.size - 1;
}

void kl_ast_del(struct kl_ast* ast) {
    struct kl_astvisit visdel;
    kl_astvisit_new(ast, NULL, del_aux, &visdel, PostOrder);
    kl_astvisit_run(&visdel);
    kl_astvisit_del(&visdel);
    abl_vec_node_del(&ast->nodes);
}
