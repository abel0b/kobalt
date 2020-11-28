#include "kobalt/ast.h"
#include "kobalt/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "kobalt/type.h"
#include "kobalt/log.h"

kbvec_impl(struct kbnode, node)

void kbast_new(struct kbast* ast) {
    kbvec_node_new(&ast->nodes);
}

void kbastvisit_new(struct kbast* ast, void* ctx, int (*visit)(struct kbastvisit *), struct kbastvisit* astvisit, enum kbastorder order) {
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

int kbastvisit_rec(struct kbastvisit* astvisit, int nid, int depth) {
    if (nid == -1) {
        return 0;
    }
    struct kbnode* node = &astvisit->ast->nodes.data[nid];
    astvisit->cur.nid = nid;
    astvisit->cur.depth = depth;

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
                kbastvisit_rec(astvisit, node->data.group.items[i], depth + 1);
            }
            break;
        case NFun:
            kbastvisit_rec(astvisit, node->data.fun.id, depth + 1);
            kbastvisit_rec(astvisit, node->data.fun.funparams, depth + 1);
            kbastvisit_rec(astvisit, node->data.fun.rettype, depth + 1);
            kbastvisit_rec(astvisit, node->data.fun.body, depth + 1);
            break;
        case NFunParam:
            kbastvisit_rec(astvisit, node->data.funparam.id, depth + 1);
            kbastvisit_rec(astvisit, node->data.funparam.type, depth + 1);
            break;
        case NCall:
            kbastvisit_rec(astvisit, node->data.call.id, depth + 1);
            kbastvisit_rec(astvisit, node->data.call.callparams, depth + 1);
            break;
        case NAssign:
            break;
        case NId:
            break;
        case NCallParam:
            kbastvisit_rec(astvisit, node->data.callparam.expr, depth + 1);
            break;
        case NIfBranch:
        case NElifBranch:
            kbastvisit_rec(astvisit, node->data.ifbranch.cond, depth + 1);
            kbastvisit_rec(astvisit, node->data.ifbranch.conseq, depth + 1);
            break;
        case NElseBranch:
            kbastvisit_rec(astvisit, node->data.ifbranch.conseq, depth + 1);
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

void kbastvisit_run(struct kbastvisit* astvisit) {
    kbastvisit_rec(astvisit, 0, 0);
}

void kbastvisit_del(struct kbastvisit* astvisit) {
    unused(astvisit);
}

struct kbast_disp_ctx {
    FILE* out;
    struct kbastinfo* astinfo;
};

static int display_aux(struct kbastvisit* astvisit) {
    struct kbnode* node = &astvisit->ast->nodes.data[astvisit->cur.nid];
    struct kbast_disp_ctx* ctx = (struct kbast_disp_ctx*)astvisit->ctx;

    indent(ctx->out, astvisit->cur.depth);
    fprintf(ctx->out, BWHT "%s" RESET " <%d:%d>", kbnode_kind_str(node->kind), node->loc.line, node->loc.col);

    switch(node->kind) {
        case NType:
            fprintf(ctx->out, " = \"%s\"", node->data.type.name);
            break;
        case NStrLit:
            fprintf(ctx->out, " = " BYEL "\"%s\"" RESET, node->data.strlit.value);
            break;
        case NIntLit:
            fprintf(ctx->out, " = " BYEL "%s" RESET, node->data.intlit.value);
            break;
        case NFloatLit:
            fprintf(ctx->out, " = " BYEL "%s" RESET, node->data.floatlit.value);
            break;
        case NCharLit:
            fprintf(ctx->out, " = " BYEL "\'%s\'" RESET, node->data.charlit.value);
            break;
        case NId:
            fprintf(ctx->out, " = %s", node->data.id.name);
            break;
        case NImport:
            fprintf(ctx->out, " = %s", node->data.import.path);
            break;
        default:
            break;
    }
    if (ctx->astinfo) {
        struct kbtype* type = ctx->astinfo->types.data[astvisit->cur.nid];
        if (type) {
            fprintf(ctx->out, " :: " GRN);
            kbtype_display(ctx->astinfo->types.data[astvisit->cur.nid]);
            fprintf(ctx->out, RESET);
        }
    }
    fprintf(ctx->out, "\n");
    return 1;
}

void kbast_display(FILE* out, struct kbast* ast, struct kbastinfo* astinfo) {
    struct kbastvisit visdisp;
    struct kbast_disp_ctx ctx = {
        .out = out,
        .astinfo = astinfo,
    };
    kbastvisit_new(ast, &ctx, display_aux, &visdisp, PreOrder);
    kbastvisit_run(&visdisp);
    kbastvisit_del(&visdisp);
}

static int del_aux(struct kbastvisit* astvisit) {
    struct kbnode* node = &astvisit->ast->nodes.data[astvisit->cur.nid];
    switch(node->kind) {
        case NCallParams:
        case NSeq:
        case NFunParams:
        case NProgram:
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
            kbfree(node->data.floatlit.value);
            break;
        case NAssign:
            break;
        case NId:
            kbfree(node->data.id.name);
            break;
        case NImport:
            kbfree(node->data.import.path);
            break;
        default:
            break;
    }
    return 1;
}

int kbast_add(struct kbast* ast, enum kbnode_kind kind, int parent, struct kbloc loc) {
    struct kbnode node = {
        .kind = kind,
        .parent = parent,
        .loc = loc,
    };
    memset(&node.data, 0, sizeof(node.data));
    kbvec_node_push(&ast->nodes, node);
    return ast->nodes.size - 1;
}

void kbast_del(struct kbast* ast) {
    struct kbastvisit visdel;
    kbastvisit_new(ast, NULL, del_aux, &visdel, PostOrder);
    kbastvisit_run(&visdel);
    kbastvisit_del(&visdel);
    kbvec_node_del(&ast->nodes);
}
