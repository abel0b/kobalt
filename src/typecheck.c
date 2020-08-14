#include "kobalt/typecheck.h"
#include "kobalt/error.h"
#include "kobalt/ast.h"
#include "kobalt/dict.h"
#include <stdio.h>
#include <assert.h>

struct kbdict global;

struct typenode {
    struct kbdict scope;
};

static int typecheck_aux(struct kbastvisitor * astvisitor) {
    struct kbnode * node = kbast_getnode(astvisitor->ast, astvisitor->cur.nid);
    // struct kbtypenode * typenode = kbdict_set(node->annotations, "typenode", malloc(sizeof));

    switch(node->kind) {
        case NFile:
            break;
        case NFun:
            break;
        case NFunParam:
            break;
        case NFunParams:
            break;
        case NFunBody:
            break;
        case NDecl:
            break;
        case NType:
            break;
        case NExpr:
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
            break;
        case NAssign:
            break;
        case NId:
            break;
        case NCallParams:
            break;
        case NCallParam:
            break;
    }
    return 1;
}

void kbtypecheck(struct kbast * ast) {
    struct kberrvec errvec = kberrvec_make();
    struct kbastvisitor astvisitor;

    kbdict_new(&global);
    
    kbastvisitor_new(ast, NULL, typecheck_aux, &astvisitor);
    kbastvisitor_run(&astvisitor);
    kbastvisitor_del(&astvisitor);
    kberrvec_del(&errvec);

    /*kbdict_display(&global);*/
    kbdict_del(&global);
}
