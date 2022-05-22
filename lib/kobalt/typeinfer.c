#include "kobalt/typeinfer.h"
#include "kobalt/astinfo.h"
#include "kobalt/error.h"
#include "kobalt/ast.h"
#include "abl/dict.h"
#include "kobalt/type.h"
#include "abl/log.h"
#include "abl/vec.h"
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

struct kl_typeinfer_ctx {
    struct abl_vec_type types;
};

static int has_scope(struct kl_node* node) {
    return node->kind == NProgram || (node->kind == NFun) || node->kind == NIfBranch || node->kind == NCase || node->kind == NForLoop;
}

static int typeinfer_rec(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid);

static void typeinfer_val(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    struct kl_mod* mod = kl_modgraph_get(modgraph, modid);
    char* name = ast->nodes.data[node->data.val.id].data.id.name;
    struct abl_str name_str;
    abl_str_new(&name_str);
    abl_str_cat(&name_str, name);

    if (kl_modgraph_try_resolve_local(modgraph, modid, nid, &name_str)) {
        abl_elog("redefinition of value '%s'", name);
        exit(1);
    }

    struct kl_symbol* val_sym = kl_modgraph_define(modgraph, modid, nid, &name_str);
    typeinfer_rec(ast, modgraph, modid, ctx, scope, node->data.val.expr);
    struct kl_type* val_type = abl_vec_type_last(&ctx->types);
    mod->astinfo.types.data[nid] = val_type;
    val_sym->type = val_type;
    val_sym->kind = ValSym;
    abl_str_del(&name_str);
}

static void typeinfer_forloop(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    struct kl_mod* mod = kl_modgraph_get(modgraph, modid);
    char* name = ast->nodes.data[node->data.forloop.id].data.id.name;
    struct abl_str name_str;
    abl_str_new(&name_str);
    abl_str_cat(&name_str, name);

    struct kl_symbol* val_sym = kl_modgraph_define(modgraph, modid, nid, &name_str);

    struct kl_type* val_type = (struct kl_type *) abl_objpool_alloc(&mod->astinfo.type_pool);
    kl_type_new(val_type, Int);
    val_sym->type = val_type;
    val_sym->kind = ValSym;

    typeinfer_rec(ast, modgraph, modid, ctx, scope, node->data.forloop.expr);
    
    struct kl_type* last_type = abl_vec_type_pop(&ctx->types);
    unused(last_type);

    struct kl_type* for_type = (struct kl_type*)abl_objpool_alloc(&mod->astinfo.type_pool);
    kl_type_new(for_type, Unit);
    mod->astinfo.types.data[nid] = for_type;
    abl_vec_type_push(&ctx->types, for_type);
    abl_str_del(&name_str);
}

static void typeinfer_fun(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    struct kl_mod* mod = kl_modgraph_get(modgraph, modid);
    char* name = ast->nodes.data[node->data.fun.id].data.id.name;
    if (strcmp(name, "main") == 0) {
        abl_elog("cannot use 'main' as function name");
        exit(1);
    }
    struct abl_str name_str;
    abl_str_new(&name_str);
    abl_str_cat(&name_str, name);
    struct kl_symbol* funsym = kl_modgraph_try_resolve(modgraph, modid, node->parent, &name_str);

    if (funsym == NULL || (funsym->kind == FunSym && !funsym->data.fun.defined)) {
        if (funsym == NULL) {
            funsym = kl_modgraph_define(modgraph, modid, node->parent, &name_str);
            funsym->kind = FunSym;
            funsym->type = (struct kl_type*) abl_objpool_alloc(&mod->astinfo.type_pool);
        }
        mod->astinfo.types.data[nid] = funsym->type;
    }
    else if (funsym->kind == FunSym && !funsym->data.fun.defined) {
        kl_todo("check if params types are compatibles");
    }
    else {
        abl_elog("redefinition of '%s'", name);
        exit(1);
    }

    typeinfer_rec(ast, modgraph, modid, ctx, scope, node->data.fun.funparams);
    
    if (node->data.fun.rettype == -1) {
        abl_elog("cannot infer return type for function '%s'; explicit type annotation is required", name);
        exit(1);
    }
    typeinfer_rec(ast, modgraph, modid, ctx, scope, node->data.fun.rettype);

    int num_params = ast->nodes.data[node->data.fun.funparams].data.group.numitems;

    struct kl_type* out_type = abl_vec_type_pop(&ctx->types);

    kl_type_new(funsym->type, Fun);
    kl_funtype_new(&funsym->type->data.fun, out_type);

    for(int i = num_params - 1; i >= 0; --i) {
        struct kl_type* in_type = ctx->types.data[ctx->types.size - 1 - i];
        kl_funtype_addparam(&funsym->type->data.fun, in_type);
    }

    abl_vec_type_resize(&ctx->types, ctx->types.size - num_params);

    if (node->data.fun.body != -1) {
        typeinfer_rec(ast, modgraph, modid, ctx, scope, node->data.fun.body);
    }

    abl_vec_type_push(&ctx->types, funsym->type);
    abl_str_del(&name_str);
}

static void typeinfer_group(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    for(int i = 0; i < node->data.group.numitems; ++i) {
        typeinfer_rec(ast, modgraph, modid, ctx, scope, node->data.group.items[i]);
    }
}

static void typeinfer_funparam(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    char* name = ast->nodes.data[node->data.funparam.id].data.id.name;
    if (node->data.funparam.type == -1) {
        abl_elog("cannot infer type for function parameter '%s'; explicit type annotation is required", name);
        exit(1);
    }
    typeinfer_rec(ast, modgraph, modid, ctx, scope, node->data.funparam.type);
    struct abl_str name_str;
    abl_str_new(&name_str);
    abl_str_cat(&name_str, name);
    struct kl_symbol* symbol = kl_modgraph_define(modgraph, modid, nid, &name_str);
    abl_str_del(&name_str);

    symbol->kind = ValSym;
    symbol->type = abl_vec_type_last(&ctx->types);
}

static void typeinfer_type(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    struct kl_mod* mod = kl_modgraph_get(modgraph, modid);
    struct kl_type* type = (struct kl_type *) abl_objpool_alloc(&mod->astinfo.type_pool);
    if (strcmp(node->data.type.name, "int") == 0) {
        kl_type_new(type, Int);
    }
    else if (strcmp(node->data.type.name, "char") == 0) {
        kl_type_new(type, Char);
    }
    else if (strcmp(node->data.type.name, "()") == 0) {
        kl_type_new(type, Unit);
    }
    else if (strcmp(node->data.type.name, "str") == 0) {
        kl_type_new(type, Str);
    }
    else {
        abl_elog("undefined type '%s'", node->data.type.name);
        exit(1);
    }
    abl_vec_type_push(&ctx->types, type);
}

static void typeinfer_seq(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid)  {
    struct kl_node* node = &ast->nodes.data[nid];
    struct kl_mod* mod = kl_modgraph_get(modgraph, modid);
    for(int i = 0; i < node->data.group.numitems - 1; ++i) {
        typeinfer_rec(ast, modgraph, modid, ctx, scope, node->data.group.items[i]);
        abl_vec_type_pop(&ctx->types);
    }
    typeinfer_rec(ast, modgraph, modid, ctx, scope, node->data.group.items[node->data.group.numitems - 1]);
    struct kl_type* seq_type = abl_vec_type_last(&ctx->types);
    mod->astinfo.types.data[nid] = seq_type;
}

static void typeinfer_callparams(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    for(int i = 0; i < node->data.group.numitems; ++ i) {
        typeinfer_rec(ast, modgraph, modid, ctx, scope, node->data.group.items[i]);
    }
}

static void typeinfer_callparam(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    typeinfer_rec(ast, modgraph, modid, ctx, scope, node->data.callparam.expr);
}

static void typeinfer_strlit(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid) {
    struct kl_mod* mod = kl_modgraph_get(modgraph, modid);
    struct kl_type* str_type = (struct kl_type *) abl_objpool_alloc(&mod->astinfo.type_pool);
    kl_type_new(str_type, Str);
    mod->astinfo.types.data[nid] = str_type;
    abl_vec_type_push(&ctx->types, str_type);
}

static void typeinfer_floatlit(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid) {
    struct kl_mod* mod = kl_modgraph_get(modgraph, modid);
    struct kl_type* float_type = (struct kl_type *) abl_objpool_alloc(&mod->astinfo.type_pool);
    kl_type_new(float_type, Float);
    mod->astinfo.types.data[nid] = float_type;
    abl_vec_type_push(&ctx->types, float_type);
}

static void typeinfer_intlit(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid) {
    struct kl_mod* mod = kl_modgraph_get(modgraph, modid);
    struct kl_type* int_type = (struct kl_type *) abl_objpool_alloc(&mod->astinfo.type_pool);
    kl_type_new(int_type, Int);
    mod->astinfo.types.data[nid] = int_type;
    abl_vec_type_push(&ctx->types, int_type);
}

static void typeinfer_charlit(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid) {
    struct kl_mod* mod = kl_modgraph_get(modgraph, modid);
    struct kl_type* char_type = (struct kl_type *) abl_objpool_alloc(&mod->astinfo.type_pool);
    kl_type_new(char_type, Char);
    mod->astinfo.types.data[nid] = char_type;
    abl_vec_type_push(&ctx->types, char_type);
}

static void typeinfer_id(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid) {
    struct kl_mod* mod = kl_modgraph_get(modgraph, modid);
    struct kl_node* node = &ast->nodes.data[nid];
    struct abl_str name_str;
    abl_str_new(&name_str);
    abl_str_cat(&name_str, node->data.id.name);
    mod->astinfo.types.data[nid] = kl_modgraph_resolve(modgraph, modid, nid, &name_str)->type;
    abl_str_del(&name_str);

    abl_vec_type_push(&ctx->types, mod->astinfo.types.data[nid]);
}

static void typeinfer_call(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    struct kl_mod* mod = kl_modgraph_get(modgraph, modid);
    typeinfer_rec(ast, modgraph, modid, ctx, scope, node->data.call.callparams);

    char* name = ast->nodes.data[node->data.call.id].data.id.name;
    struct abl_str name_str;
    abl_str_new(&name_str);
    abl_str_cat(&name_str, name);
    struct kl_type* type = kl_modgraph_resolve(modgraph, modid, nid, &name_str)->type;
    abl_str_del(&name_str);

    for(int i = 0; i < ast->nodes.data[node->data.call.callparams].data.group.numitems; ++ i) {
        abl_vec_type_pop(&ctx->types);
    }

    assert(type->kind == Fun);
    abl_vec_type_push(&ctx->types, type->data.fun.out_type);

    mod->astinfo.types.data[nid] = type->data.fun.out_type;
}

static void typeinfer_ifelse(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    struct kl_mod* mod = kl_modgraph_get(modgraph, modid);
    for(int i = 0; i < node->data.group.numitems - 1; ++i) {
        typeinfer_rec(ast, modgraph, modid, ctx, scope, node->data.group.items[i]);
        abl_vec_type_pop(&ctx->types);
    }
    typeinfer_rec(ast, modgraph, modid, ctx, scope, node->data.group.items[node->data.group.numitems - 1]);
    struct kl_type* ifelse_type = abl_vec_type_last(&ctx->types);
    mod->astinfo.types.data[nid] = ifelse_type;
}

static void typeinfer_ifbranch(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
	typeinfer_rec(ast, modgraph, modid, ctx, scope, node->data.ifbranch.cond);
    struct kl_type* cond_type = abl_vec_type_pop(&ctx->types);
    assert(cond_type);

	typeinfer_rec(ast, modgraph, modid, ctx, scope, node->data.ifbranch.conseq);
    // struct kl_type* conseq_type = abl_vec_type_pop(&ctx->types);
}

static void typeinfer_elsebranch(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* scope, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
	typeinfer_rec(ast, modgraph, modid, ctx, scope, node->data.ifbranch.conseq);
}

static int typeinfer_rec(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_typeinfer_ctx* ctx, struct kl_scope* pscope, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    struct kl_mod* mod = kl_modgraph_get(modgraph, modid);

#if DEBUG
    if (getenv("DEBUG_TYPEINFER")) {
        abl_dlog("typeinfer %s", kl_node_kind_str(node->kind));
    }
#endif

    struct kl_scope* scope = pscope;
    if (has_scope(node)) {
        scope = kl_astinfo_alloc_scope(&mod->astinfo, pscope);
    }
    
    mod->astinfo.scopes.data[nid] = scope;

    switch(node->kind) {
        case NProgram:
        case NFunParams:
            typeinfer_group(ast, modgraph, modid, ctx, scope, nid);
            break;
        case NFun:
            typeinfer_fun(ast, modgraph, modid, ctx, scope, nid);
            break;
        case NFunParam:
            typeinfer_funparam(ast, modgraph, modid, ctx, scope, nid);
            break;
        case NType:
            typeinfer_type(ast, modgraph, modid, ctx, scope, nid);
            break;
        case NSeq:
            typeinfer_seq(ast, modgraph, modid, ctx, scope, nid);
            break;
        case NCallParams:
            typeinfer_callparams(ast, modgraph, modid, ctx, scope, nid);
            break;
        case NCallParam:
            typeinfer_callparam(ast, modgraph, modid, ctx, scope, nid);
            break;
        case NStrLit:
            typeinfer_strlit(ast, modgraph, modid, ctx, scope, nid);
            break;
        case NIntLit:
            typeinfer_intlit(ast, modgraph, modid, ctx, scope, nid);
            break;
        case NFloatLit:
            typeinfer_floatlit(ast, modgraph, modid, ctx, scope, nid);
            break;
        case NCharLit:
            typeinfer_charlit(ast, modgraph, modid, ctx, scope, nid);
            break;
        case NId:
			typeinfer_id(ast, modgraph, modid, ctx, scope, nid);
            break;
        case NCall:
			typeinfer_call(ast, modgraph, modid, ctx, scope, nid);
            break;
        case NVal:
			typeinfer_val(ast, modgraph, modid, ctx, scope, nid);
            break;
        case NForLoop:
            typeinfer_forloop(ast, modgraph, modid, ctx, scope, nid);
            break;
        case NIfElse:
			typeinfer_ifelse(ast, modgraph, modid, ctx, scope, nid);
            break;
        case NIfBranch:
        case NElifBranch:
			typeinfer_ifbranch(ast, modgraph, modid, ctx, scope, nid);
            break;
        case NElseBranch:
			typeinfer_elsebranch(ast, modgraph, modid, ctx, scope, nid);
            break;
        default:
            break;
    }

    // abl_dlog("scope %s", kl_node_kind_str(node->kind));
    // abl_dict_display(&scope->data);
    return 1;
}

void kl_typeinfer(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid) {
    struct kl_typeinfer_ctx ctx;
    abl_vec_type_new(&ctx.types);
    
    typeinfer_rec(ast, modgraph, modid, &ctx, NULL, 0);

    // struct kl_type* type = (struct kl_type*)kl_modgraph_resolve(astinfo, "writeln", 0);
    // abl_ilog("RESOLVED OK %s %p", "writeln", type);
    // exit(0);

    abl_vec_type_del(&ctx.types);
}
