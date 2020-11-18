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
    struct kbvec_type types;
};

static int has_scope(struct kbnode* node) {
    return node->kind == NProgram || (node->kind == NFun) || node->kind == NIfBranch || node->kind == NCase;
}

static int typeinfer_rec(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* pscope, int nid);

static void typeinfer_fun(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbnode* node = &ast->nodes.data[nid];
    char* name = ast->nodes.data[node->data.fun.id].data.id.name;
    if (strcmp(name, "main") == 0) {
        kbelog("cannot use 'main' as function name");
        exit(1);
    }
    struct kbsymbol* funsym = kbscope_try_resolve(astinfo, name, node->parent);

    if (funsym == NULL || (funsym->kind == FunSym && !funsym->data.fun.defined)) {
        if (funsym == NULL) {
            funsym = kbscope_define(astinfo, name, node->parent);
            funsym->kind = FunSym;
            funsym->type = (struct kbtype *) kbobjpool_alloc(&astinfo->type_pool);
        }
        astinfo->types.data[nid] = funsym->type;
    }
    else if (funsym->kind == FunSym && !funsym->data.fun.defined) {
        kb_todo("check if params types are compatibles");
    }
    else {
        kbelog("redefinition of '%s'", name);
        exit(1);
    }

    typeinfer_rec(ast, astinfo, ctx, scope, node->data.fun.funparams);
    
    if (node->data.fun.rettype == -1) {
        kbelog("cannot infer return type for function '%s'; explicit type annotation is required", name);
        exit(1);
    }
    typeinfer_rec(ast, astinfo, ctx, scope, node->data.fun.rettype);

    int num_params = ast->nodes.data[node->data.fun.funparams].data.group.numitems;

    struct kbtype* out_type = kbvec_type_pop(&ctx->types);

    kbtype_new(funsym->type, Fun);
    kbfuntype_new(&funsym->type->data.fun, out_type);

    for(int i = num_params - 1; i >= 0; --i) {
        struct kbtype* in_type = ctx->types.data[ctx->types.size - 1 - i];
        kbfuntype_addparam(&funsym->type->data.fun, in_type);
    }

    kbvec_type_resize(&ctx->types, ctx->types.size - num_params);

    if (node->data.fun.body != -1) {
        typeinfer_rec(ast, astinfo, ctx, scope, node->data.fun.body);
    }

    kbvec_type_push(&ctx->types, funsym->type);
}

static void typeinfer_group(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbnode* node = &ast->nodes.data[nid];
    for(int i = 0; i < node->data.group.numitems; ++i) {
        typeinfer_rec(ast, astinfo, ctx, scope, node->data.group.items[i]);
    }
}

static void typeinfer_funparam(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbnode* node = &ast->nodes.data[nid];
    char* name = ast->nodes.data[node->data.funparam.id].data.id.name;
    if (node->data.funparam.type == -1) {
        kbelog("cannot infer type for function parameter '%s'; explicit type annotation is required", name);
        exit(1);
    }
    typeinfer_rec(ast, astinfo, ctx, scope, node->data.funparam.type);
    struct kbsymbol* symbol = kbscope_define(astinfo, name, nid);
    symbol->kind = ValSym;
    symbol->type = kbvec_type_last(&ctx->types);
}

static void typeinfer_type(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbnode* node = &ast->nodes.data[nid];
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
    else if (strcmp(node->data.type.name, "str") == 0) {
        kbtype_new(type, Str);
    }
    else {
        kbelog("undefined type '%s'", node->data.type.name);
        exit(1);
    }
    kbvec_type_push(&ctx->types, type);
}

static void typeinfer_seq(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbnode* node = &ast->nodes.data[nid];
    for(int i = 0; i < node->data.group.numitems - 1; ++i) {
        typeinfer_rec(ast, astinfo, ctx, scope, node->data.group.items[i]);
        kbvec_type_pop(&ctx->types);
    }
    typeinfer_rec(ast, astinfo, ctx, scope, node->data.group.items[node->data.group.numitems - 1]);
    struct kbtype* seq_type = kbvec_type_last(&ctx->types);
    astinfo->types.data[nid] = seq_type;
}

static void typeinfer_callparams(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbnode* node = &ast->nodes.data[nid];
    for(int i = 0; i < node->data.group.numitems; ++ i) {
        typeinfer_rec(ast, astinfo, ctx, scope, node->data.group.items[i]);
    }
}

static void typeinfer_callparam(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbnode* node = &ast->nodes.data[nid];
    typeinfer_rec(ast, astinfo, ctx, scope, node->data.callparam.expr);
}

static void typeinfer_strlit(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbtype* str_type = (struct kbtype *) kbobjpool_alloc(&astinfo->type_pool);
    kbtype_new(str_type, Str);
    astinfo->types.data[nid] = str_type;
    kbvec_type_push(&ctx->types, str_type);
}

static void typeinfer_floatlit(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbtype* float_type = (struct kbtype *) kbobjpool_alloc(&astinfo->type_pool);
    kbtype_new(float_type, Float);
    astinfo->types.data[nid] = float_type;
    kbvec_type_push(&ctx->types, float_type);
}

static void typeinfer_intlit(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbtype* int_type = (struct kbtype *) kbobjpool_alloc(&astinfo->type_pool);
    kbtype_new(int_type, Int);
    astinfo->types.data[nid] = int_type;
    kbvec_type_push(&ctx->types, int_type);
}

static void typeinfer_charlit(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbtype* char_type = (struct kbtype *) kbobjpool_alloc(&astinfo->type_pool);
    kbtype_new(char_type, Char);
    astinfo->types.data[nid] = char_type;
    kbvec_type_push(&ctx->types, char_type);
}

static void typeinfer_id(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbnode* node = &ast->nodes.data[nid];
    astinfo->types.data[nid] = kbscope_resolve(astinfo, node->data.id.name, nid)->type;
    kbvec_type_push(&ctx->types, astinfo->types.data[nid]);
}

static void typeinfer_call(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbnode* node = &ast->nodes.data[nid];
	typeinfer_rec(ast, astinfo, ctx, scope, node->data.call.callparams);

    char* name = ast->nodes.data[node->data.call.id].data.id.name;
    struct kbtype* type = kbscope_resolve(astinfo, name, nid)->type;
    assert(type->kind == Fun);
    kbvec_type_push(&ctx->types, type->data.fun.out_type);

    astinfo->types.data[nid] = type->data.fun.out_type;
}

static void typeinfer_ifelse(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbnode* node = &ast->nodes.data[nid];
    for(int i = 0; i < node->data.group.numitems - 1; ++i) {
        typeinfer_rec(ast, astinfo, ctx, scope, node->data.group.items[i]);
        kbvec_type_pop(&ctx->types);
    }
    typeinfer_rec(ast, astinfo, ctx, scope, node->data.group.items[node->data.group.numitems - 1]);
    struct kbtype* ifelse_type = kbvec_type_last(&ctx->types);
    astinfo->types.data[nid] = ifelse_type;
}

static void typeinfer_ifbranch(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbnode* node = &ast->nodes.data[nid];
	typeinfer_rec(ast, astinfo, ctx, scope, node->data.ifbranch.cond);
    struct kbtype* cond_type = kbvec_type_pop(&ctx->types);
    assert(cond_type);

	typeinfer_rec(ast, astinfo, ctx, scope, node->data.ifbranch.conseq);
    // struct kbtype* conseq_type = kbvec_type_pop(&ctx->types);
}

static void typeinfer_elsebranch(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* scope, int nid) {
    struct kbnode* node = &ast->nodes.data[nid];
	typeinfer_rec(ast, astinfo, ctx, scope, node->data.ifbranch.conseq);
}

static int typeinfer_rec(struct kbast* ast, struct kbastinfo* astinfo, struct kbtypeinfer_ctx* ctx, struct kbscope* pscope, int nid) {
    struct kbnode* node = &ast->nodes.data[nid];
#if DEBUG
    if (getenv("DEBUG_TYPEINFER")) {
        kbdlog("typeinfer %s", kbnode_kind_str(node->kind));
    }
#endif

    struct kbscope* scope = pscope;
    if (has_scope(node)) {
        scope = kbastinfo_alloc_scope(astinfo, pscope);
    }
    
    astinfo->scopes.data[nid] = scope;

    switch(node->kind) {
        case NProgram:
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
        case NSeq:
            typeinfer_seq(ast, astinfo, ctx, scope, nid);
            break;
        case NCallParams:
            typeinfer_callparams(ast, astinfo, ctx, scope, nid);
            break;
        case NCallParam:
            typeinfer_callparam(ast, astinfo, ctx, scope, nid);
            break;
        case NStrLit:
            typeinfer_strlit(ast, astinfo, ctx, scope, nid);
            break;
        case NIntLit:
            typeinfer_intlit(ast, astinfo, ctx, scope, nid);
            break;
        case NFloatLit:
            typeinfer_floatlit(ast, astinfo, ctx, scope, nid);
            break;
        case NCharLit:
            typeinfer_charlit(ast, astinfo, ctx, scope, nid);
            break;
        case NId:
			typeinfer_id(ast, astinfo, ctx, scope, nid);
            break;
        case NCall:
			typeinfer_call(ast, astinfo, ctx, scope, nid);
            break;
        case NIfElse:
			typeinfer_ifelse(ast, astinfo, ctx, scope, nid);
            break;
        case NIfBranch:
        case NElifBranch:
			typeinfer_ifbranch(ast, astinfo, ctx, scope, nid);
            break;
        case NElseBranch:
			typeinfer_elsebranch(ast, astinfo, ctx, scope, nid);
            break;
        default:
            break;
    }

    // kbdlog("scope %s", kbnode_kind_str(node->kind));
    // kbdict_display(&scope->data);
    return 1;
}

void kbtypeinfer(struct kbast* ast, struct kbastinfo* astinfo) {
    struct kberrvec errvec = kberrvec_make();

    struct kbtypeinfer_ctx ctx;
    kbvec_type_new(&ctx.types);

    kbastinfo_new(astinfo);
    kbvec_scope_resize(&astinfo->scopes, ast->nodes.size);
    kbvec_type_resize(&astinfo->types, ast->nodes.size);
    kbvec_type_fill(&astinfo->types, NULL);
    typeinfer_rec(ast, astinfo, &ctx, NULL, 0);

    // struct kbtype* type = (struct kbtype*)kbscope_resolve(astinfo, "writeln", 0);
    // kbilog("RESOLVED OK %s %p", "writeln", type);
    // exit(0);

    kbvec_type_del(&ctx.types);

    kberrvec_del(&errvec);
}
