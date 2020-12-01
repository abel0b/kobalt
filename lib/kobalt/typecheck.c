#include "kobalt/typecheck.h"
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

static int typecheck_rec(struct kbast* ast, struct kbmodgraph* modgraph, struct kbstr* modid, int nid);

static void typecheck_group(struct kbast* ast, struct kbmodgraph* modgraph, struct kbstr* modid, int nid) { 
    struct kbnode* node = &ast->nodes.data[nid];
    for(int i = 0; i < node->data.group.numitems; ++i) {
        typecheck_rec(ast, modgraph, modid, node->data.group.items[i]);
    }
}

static void typecheck_call(struct kbast* ast, struct kbmodgraph* modgraph, struct kbstr* modid, int nid) { 
    struct kbnode* node = &ast->nodes.data[nid];
    struct kbmod* mod = kbmodgraph_get(modgraph, modid);
    char* name = ast->nodes.data[node->data.call.id].data.id.name;
    struct kbstr name_str;
    kbstr_new(&name_str);
    kbstr_cat(&name_str, name);
    struct kbtype* funtype = kbmodgraph_resolve(modgraph, modid, nid, &name_str)->type;
    kbstr_del(&name_str);
    // kbilog("funtype %p", funtype);
    // assert(funtype->kind == Fun);

    if (funtype->data.fun.in_types.size < ast->nodes.data[node->data.call.callparams].data.group.numitems) {
        kbelog("too few parameters for function '%s'", ast->nodes.data[node->data.call.id].data.id.name);
        kbelog("expected %d but got %d", funtype->data.fun.in_types.size, ast->nodes.data[node->data.call.callparams].data.group.numitems);
        exit(1);
    }
    else if (funtype->data.fun.in_types.size > ast->nodes.data[node->data.call.callparams].data.group.numitems) {
        kbelog("too many parameters for function '%s'", ast->nodes.data[node->data.call.id].data.id.name);
        kbelog("expected %d but got %d", funtype->data.fun.in_types.size, ast->nodes.data[node->data.call.callparams].data.group.numitems);
        exit(1);
    }

    for(int i = 0; i < funtype->data.fun.in_types.size; ++i) {
        if (funtype->data.fun.in_types.data[i]->kind != mod->astinfo.types.data[ast->nodes.data[ast->nodes.data[node->data.call.callparams].data.group.items[i]].data.callparam.expr]->kind) {
            kbelog("incompatible parameter %d for function '%s'", i+1, ast->nodes.data[node->data.call.id].data.id.name);
            kbelog("expected %d but got %d", funtype->data.fun.in_types.data[i]->kind, mod->astinfo.types.data[ast->nodes.data[ast->nodes.data[node->data.call.callparams].data.group.items[i]].data.callparam.expr]->kind);
            exit(1);
        }
    }
    typecheck_rec(ast, modgraph, modid, node->data.call.callparams);
}

static void typecheck_fun(struct kbast* ast, struct kbmodgraph* modgraph, struct kbstr* modid, int nid) { 
    struct kbnode* node = &ast->nodes.data[nid];

}

static void typecheck_ifelse(struct kbast* ast, struct kbmodgraph* modgraph, struct kbstr* modid, int nid) { 
    struct kbnode* node = &ast->nodes.data[nid];

}

static void typecheck_ifbranch(struct kbast* ast, struct kbmodgraph* modgraph, struct kbstr* modid, int nid) { 
    struct kbnode* node = &ast->nodes.data[nid];

}

static void typecheck_elsebranch(struct kbast* ast, struct kbmodgraph* modgraph, struct kbstr* modid, int nid) { 
    struct kbnode* node = &ast->nodes.data[nid];

}

static int typecheck_rec(struct kbast* ast, struct kbmodgraph* modgraph, struct kbstr* modid, int nid) {
    struct kbnode* node = &ast->nodes.data[nid];
#if DEBUG
    if (getenv("DEBUG_TYPECHECK")) {
        kbdlog("typecheck %s", kbnode_kind_str(node->kind));
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

void kbtypecheck(struct kbast* ast, struct kbmodgraph* modgraph, struct kbstr* modid) {
    typecheck_rec(ast, modgraph, modid, 0);
}
