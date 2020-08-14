#include "kobalt/syntax.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char * kbnode_kind_str(enum kbnode_kind kind) {
    switch(kind) {
        case NFile:
            return "File";
        case NFun:
            return "Fun";
        case NFunParams:
            return "FunParams";
        case NFunParam:
            return "FunParam";
        case NFunBody:
            return "FunBody";
        case NDecl:
            return "Decl";
        case NType:
            return "Type";
        case NExpr:
            return "Expr";
        case NTerm:
            return "Term";
        case NBinExpr:
            return "BinExpr";
        case NStrLit:
            return "StrLit";
        case NIntLit:
            return "IntLit";
        case NFloatLit:
            return "FloatLit";
        case NCharLit:
            return "CharLit";
        case NCall:
            return "Call";
        case NCallParams:
            return "CallParams";
        case NCallParam:
            return "CallParam";
        case NAssign:
            return "Assign";
        case NId:
            return "Id";
    }
    return "UNDEFINED";
}

int isgroup(struct kbnode *node) {
    return node->kind == NFile || node->kind == NFunParams || node->kind == NFunBody || node->kind == NCallParams;
}
