#include "kobalt/ast.h"
#include <stdio.h>

static void indent(int level) {
    for (int ii=0; ii<=level; ++ii) {
        if (ii<level) {
            printf("   ");
        }
        else {
            printf("+--");
        }
    }
}

static void kbast_display_rec(struct kbnode * node, int level) {
    if(node == NULL) return;
    indent(level);
    switch(node->kind) {
        case NPROG:
            printf("prog\n");
            for(int ii=0; ii<node->data.prog.numitems; ++ii) {
                kbast_display_rec(node->data.prog.items[ii], level+1);
            }
            break;
        case NITEM:
            printf("item\n");
            break;
        case NFUN:
            printf("fun\n");
            for(int ii=0; ii<node->data.fun.numargs; ++ii) {
                kbast_display_rec(node->data.fun.args[ii], level+1);
            }
            kbast_display_rec(node->data.fun.funbody, level+1);
            break;
        case NARG:
            printf("arg\n");
            kbast_display_rec(node->data.arg.id, level+1);
            kbast_display_rec(node->data.arg.type, level+1);
            break;
        case NFUNBODY:
            printf("funbody\n");
            for(int ii=0; ii<node->data.funbody.numexprs; ++ii) {
                kbast_display_rec(node->data.funbody.exprs[ii], level+1);
            }
            break;
        case NDECL:
            printf("decl\n");
            break;
        case NTYPE:
            printf("type\n");
            break;
        case NEXPR:
            printf("expr\n");
            kbast_display_rec(node->data.expr.expr, level+1);
            break;
        case NTERM:
            printf("term\n");
            break;
        case NBINEXPR:
            printf("binexpr\n");
            break;
        case NLIT:
            printf("lit\n");
            kbast_display_rec(node->data.literal.value, level+1);
            break;
        case NFUNCALL:
            printf("funcall\n");
            kbast_display_rec(node->data.funcall.id, level+1);
            break;
        case NASSIGN:
            printf("assign\n");
            break;
        case NID:
            printf("id \"%s\"\n", node->data.id.name);
            break;
    }
}

void kbast_display(struct kbnode * node) {
    kbast_display_rec(node, 0); 
}
