#include "kobalt/typecheck.h"
#include "kobalt/error.h"
#include "kobalt/ast.h"
#include "kobalt/annot.h"
#include "kobalt/dict.h"
#include "kobalt/type.h"
#include "kobalt/log.h"
#include "kobalt/vec.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

struct typecheck {
    struct kbvec types;
};

void kbtyping_new(void* typing) {
    unused(typing);
}

void kbtyping_del(void* typing) {
    unused(typing);
}

void kbscope_new(void* scope) {
    kbdict_new((struct kbdict*) scope);
}

void kbscope_del(void* scope) {
    kbdict_del((struct kbdict*) scope);
}

static int isblocknode(struct kbnode* node) {
    return node->kind == NFile || node->kind == NFun || node->kind == NIfBranch || node->kind == NCase;
}

static int typecheck_aux(struct kbastvisitor* astvisitor) {
    struct kbnode* node = kbast_getnode(astvisitor->ast, astvisitor->cur.nid);
    struct typecheck* ctx = (struct typecheck*) astvisitor->ctx;

    // printf("NODE %s %d\n", kbnode_kind_str(node->kind), astvisitor->curop);

    if (astvisitor->curop == PreOrder && isblocknode(node)) {
        struct kbdict* scope = kbast_getannot(astvisitor->ast, astvisitor->cur.nid, AnnotScope);
        unused(scope);
    }
    
    switch(node->kind) {
        case NFun:
            if (astvisitor->curop == PreOrder) {
                char* name = astvisitor->ast->nodes[node->data.fun.id].data.id.name;
                if (kbscope_isdef(astvisitor->ast, astvisitor->cur.nid, name)) {
                    kbelog("redefinition of '%s'", name);
                    exit(1);
                }
                else {
                    struct kbdict* scope = kbscope_get(astvisitor->ast, astvisitor->cur.nid);
                    struct kbtype* type = (struct kbtype *) kbobjpool_alloc(&astvisitor->ast->type_pool);
                    kbdict_set(scope, name, (void*) type);
                }
            }
            else if (astvisitor->curop == PostOrder) {
                struct kbdict* scope = kbscope_get(astvisitor->ast, astvisitor->cur.nid);
                char* name = astvisitor->ast->nodes[node->data.fun.id].data.id.name;
                struct kbtype* type = (struct kbtype*) kbdict_get(scope, name);

                int num_params = kbast_getnode(astvisitor->ast, node->data.fun.funparams)->data.group.numitems;

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
            break;
        case NFunParam:
            if (astvisitor->curop == PostOrder) {
                struct kbdict* scope = kbscope_get(astvisitor->ast, astvisitor->cur.nid);
                char* name = astvisitor->ast->nodes[node->data.funparam.id].data.id.name;
                kbdict_set(scope, name, kbvec_last(&ctx->types));
            }
            break;
        case NType:
            if (astvisitor->curop == PreOrder) {
                struct kbtype* type = (struct kbtype *) kbobjpool_alloc(&astvisitor->ast->type_pool);
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
                    kbelog("unknown type name '%s'", node->data.type.name);
                    exit(1);
                }
                kbvec_push(&ctx->types, &type);
            }
            break;
        default:
            break;
    }
    return 1;
}

void kbtypecheck(struct kbast* ast) {
    kbannotctx_add(&ast->annotctx, AnnotTyping, sizeof(struct kbtyping), kbtyping_new, kbtyping_del);
    kbannotctx_add(&ast->annotctx, AnnotScope, sizeof(struct kbdict), kbscope_new, kbscope_del);

    struct kberrvec errvec = kberrvec_make();
    struct kbastvisitor astvisitor;

    struct typecheck typecheckctx;
    kbvec_new(&typecheckctx.types, sizeof(struct kbtype*));

    kbastvisitor_new(ast, &typecheckctx, typecheck_aux, &astvisitor, MixOrder);
    kbastvisitor_run(&astvisitor);
    kbastvisitor_del(&astvisitor);

    kbvec_del(&typecheckctx.types);

    kberrvec_del(&errvec);
}
