#include "kobalt/typeinfer.h"
#include "kobalt/astinfo.h"
#include "kobalt/error.h"
#include "kobalt/ast.h"
#include "kobalt/dict.h"
#include "kobalt/type.h"
#include "kobalt/log.h"
#include "kobalt/vec.h"
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

struct kbtypeinfer_ctx {
    struct kbvec types;
};

static int isblocknode(struct kbnode* node) {
    return node->kind == NFile || node->kind == NFun || node->kind == NIfBranch || node->kind == NCase;
}

static int typeinfer_rec(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* pscope, int nid);

static void typeinfer_fun(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbnode* node = &ast->nodes[nid];
    char* name = ast->nodes[node->data.fun.id].data.id.name;
    if (kbscope_resolve(astinfo, (uintptr_t)name, nid)) {
        kbelog("redefinition of '%s'", name);
        exit(1);
    }
    else {
        struct kbtype* type = (struct kbtype *)kbobjpool_alloc(&astinfo->type_pool);
        kbscope_define(scope, (uintptr_t)name, (void*)type);
    }

    typeinfer_rec(ast, astinfo, ctx, scope, node->data.fun.funparams);
    typeinfer_rec(ast, astinfo, ctx, scope, node->data.fun.rettype);

    struct kbtype* type = (struct kbtype*) kbscope_resolve(astinfo, (uintptr_t)name, nid);
    assert(type != NULL);

    int num_params = ast->nodes[node->data.fun.funparams].data.group.numitems;

    struct kbtype* out_type;
    kbvec_pop(&ctx->types, &out_type);

    kbtype_new(type, Fun);
    kbfuntype_new(&type->data.fun, out_type);

    for(int i = 0; i < num_params; ++i) {
        struct kbtype* in_type = NULL;
        kbvec_pop(&ctx->types, &in_type);
        kbfuntype_addparam(&type->data.fun, in_type);
    }

    printf("val %s : ", name);
    kbtype_display(type);
    printf("\n");
}

static void typeinfer_group(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbnode* node = &ast->nodes[nid];
    for(int i = 0; i < node->data.group.numitems; ++i) {
        typeinfer_rec(ast, astinfo, ctx, scope, node->data.group.items[i]);
    }
}

static void typeinfer_funparam(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbnode* node = &ast->nodes[nid];
    typeinfer_rec(ast, astinfo, ctx, scope, node->data.funparam.type);
    char* name = ast->nodes[node->data.funparam.id].data.id.name;
    if (strcmp(name, "_") != 0) {
        kbscope_define(scope, (uintptr_t)name, kbvec_last(&ctx->types));
    }
}

static void typeinfer_type(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbnode* node = &ast->nodes[nid];
    struct kbtype* type = (struct kbtype *) kbobjpool_alloc(&astinfo->type_pool);
    if (strcmp(node->data.type.name, "int") == 0) {
        kbtype_new(type, Int);
    }
    else if (strcmp(node->data.type.name, "char") == 0) {
        kbtype_new(type, Char);
    }
    else if (strcmp(node->data.type.name, "()") == 0) {
        kbtype_new(type, Unit);
    }
    else {
        kbelog("undefined type '%s'", node->data.type.name);
        exit(1);
    }
    kbvec_push(&ctx->types, &type);
}

static int typeinfer_rec(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* pscope, int nid) {
    struct kbnode* node = &ast->nodes[nid];
#if DEBUG
    kbdlog("typeinfer %s", kbnode_kind_str(node->kind));
#endif

    struct kbscope* scope = pscope;
    if (isblocknode(node)) {
        scope = kbastinfo_alloc_scope(astinfo, pscope);
    }
    kbdict_set(&astinfo->scopes, (uintptr_t)nid, scope);

    switch(node->kind) {
        case NFile:
        case NSeq:
        case NFunParams:
            typeinfer_group(ast, astinfo, ctx, scope, nid);
            break;
        case NFun:
            typeinfer_fun(ast, astinfo, ctx, scope, nid);
            break;
        case NFunParam:
            typeinfer_funparam(ast, astinfo, ctx, scope, nid);
            break;
        case NType:
            typeinfer_type(ast, astinfo, ctx, scope, nid);
            break;
        default:
            break;
    }
    return 1;
}

void kbtypeinfer(struct kbast* ast, struct kbastinfo* astinfo) {
    struct kberrvec errvec = kberrvec_make();

    struct kbtypeinfer_ctx ctx;
    kbvec_new(&ctx.types, sizeof(struct kbtype*));

    kbastinfo_new(astinfo);
    typeinfer_rec(ast, astinfo, &ctx, NULL, 0);

    for(int i = 0; i < ctx.types.num_elems; ++i) {
        kbtype_del(*(struct kbtype**)kbvec_get(&ctx.types, i));
    }

    kbvec_del(&ctx.types);

    kberrvec_del(&errvec);
}
