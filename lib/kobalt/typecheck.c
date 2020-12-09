#include "kobalt/typecheck.h"
#include "kobalt/astinfo.h"
#include "kobalt/error.h"
#include "kobalt/ast.h"
#include "klbase/dict.h"
#include "kobalt/type.h"
#include "klbase/log.h"
#include "klbase/vec.h"
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

static int typecheck_rec(struct kl_ast* ast, struct kl_modgraph* modgraph, struct kl_str* modid, int nid);

static void typecheck_group(struct kl_ast* ast, struct kl_modgraph* modgraph, struct kl_str* modid, int nid) { 
    struct kl_node* node = &ast->nodes.data[nid];
    for(int i = 0; i < node->data.group.numitems; ++i) {
        typecheck_rec(ast, modgraph, modid, node->data.group.items[i]);
    }
}

static void typecheck_call(struct kl_ast* ast, struct kl_modgraph* modgraph, struct kl_str* modid, int nid) { 
    struct kl_node* node = &ast->nodes.data[nid];
    struct kl_mod* mod = kl_modgraph_get(modgraph, modid);
    char* name = ast->nodes.data[node->data.call.id].data.id.name;
    struct kl_str name_str;
    kl_str_new(&name_str);
    kl_str_cat(&name_str, name);
    struct kl_type* funtype = kl_modgraph_resolve(modgraph, modid, nid, &name_str)->type;
    kl_str_del(&name_str);
    // kl_ilog("funtype %p", funtype);
    // assert(funtype->kind == Fun);

    if (funtype->data.fun.in_types.size < ast->nodes.data[node->data.call.callparams].data.group.numitems) {
        kl_elog("too few parameters for function '%s'", ast->nodes.data[node->data.call.id].data.id.name);
        kl_elog("expected %d but got %d", funtype->data.fun.in_types.size, ast->nodes.data[node->data.call.callparams].data.group.numitems);
        exit(1);
    }
    else if (funtype->data.fun.in_types.size > ast->nodes.data[node->data.call.callparams].data.group.numitems) {
        kl_elog("too many parameters for function '%s'", ast->nodes.data[node->data.call.id].data.id.name);
        kl_elog("expected %d but got %d", funtype->data.fun.in_types.size, ast->nodes.data[node->data.call.callparams].data.group.numitems);
        exit(1);
    }

    for(int i = 0; i < funtype->data.fun.in_types.size; ++i) {
        if (funtype->data.fun.in_types.data[i]->kind != mod->astinfo.types.data[ast->nodes.data[ast->nodes.data[node->data.call.callparams].data.group.items[i]].data.callparam.expr]->kind) {
            kl_elog("incompatible parameter %d for function '%s'", i+1, ast->nodes.data[node->data.call.id].data.id.name);
            kl_elog("expected %d but got %d", funtype->data.fun.in_types.data[i]->kind, mod->astinfo.types.data[ast->nodes.data[ast->nodes.data[node->data.call.callparams].data.group.items[i]].data.callparam.expr]->kind);
            exit(1);
        }
    }
    typecheck_rec(ast, modgraph, modid, node->data.call.callparams);
}

static void typecheck_fun(struct kl_ast* ast, struct kl_modgraph* modgraph, struct kl_str* modid, int nid) { 
    struct kl_node* node = &ast->nodes.data[nid];

}

static void typecheck_ifelse(struct kl_ast* ast, struct kl_modgraph* modgraph, struct kl_str* modid, int nid) { 
    struct kl_node* node = &ast->nodes.data[nid];

}

static void typecheck_ifbranch(struct kl_ast* ast, struct kl_modgraph* modgraph, struct kl_str* modid, int nid) { 
    struct kl_node* node = &ast->nodes.data[nid];

}

static void typecheck_elsebranch(struct kl_ast* ast, struct kl_modgraph* modgraph, struct kl_str* modid, int nid) { 
    struct kl_node* node = &ast->nodes.data[nid];

}

static int typecheck_rec(struct kl_ast* ast, struct kl_modgraph* modgraph, struct kl_str* modid, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
#if DEBUG
    if (getenv("DEBUG_TYPECHECK")) {
        kl_dlog("typecheck %s", kl_node_kind_str(node->kind));
    }
#endif

    switch(node->kind) {
        case NProgram:
        case NSeq:
            typecheck_group(ast, modgraph, modid, nid);
            break;
        case NFun:
            typecheck_fun(ast, modgraph, modid, nid);
            break;
        case NCall:
			typecheck_call(ast, modgraph, modid, nid);
            break;
        case NIfElse:
			typecheck_ifelse(ast, modgraph, modid, nid);
            break;
        case NIfBranch:
        case NElifBranch:
			typecheck_ifbranch(ast, modgraph, modid, nid);
            break;
        case NElseBranch:
			typecheck_elsebranch(ast, modgraph, modid, nid);
            break;
        default:
            break;
    }
    return 1;
}

void kl_typecheck(struct kl_ast* ast, struct kl_modgraph* modgraph, struct kl_str* modid) {
    typecheck_rec(ast, modgraph, modid, 0);
}
