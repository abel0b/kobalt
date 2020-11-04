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


static int typecheck_rec(struct kbast* ast, struct kbastinfo* astinfo, int nid);

static void typecheck_call(struct kbast* ast, struct kbastinfo* astinfo, int nid) { 
    struct kbnode* node = &ast->nodes[nid];
    char* name = ast->nodes[node->data.call.id].data.id.name;
    struct kbtype* type = *(struct kbtype**)kbscope_resolve(astinfo, (uintptr_t)name, nid);
    
    for(int i = 0; i < type->data.fun.in_types.num_elems; ++i) {
        struct kbtype* intype = *(struct kbtype**)kbscope_resolve(astinfo, (uintptr_t)name, nid);
    }
}

static int typecheck_rec(struct kbast* ast, struct kbastinfo* astinfo, int nid) {
    struct kbnode* node = &ast->nodes[nid];
#if DEBUG
    kbdlog("typecheck %s", kbnode_kind_str(node->kind));
#endif

    switch(node->kind) {
        case NFun:
            typecheck_call(ast, astinfo, nid);
            break;
        default:
            break;
    }
    return 1;
}

void kbtypecheck(struct kbast* ast, struct kbastinfo* astinfo) {
    typecheck_rec(ast, astinfo, 0);
}
