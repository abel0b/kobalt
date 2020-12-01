#include "kobalt/syntax.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char* kbnode_kind_str(enum kbnode_kind kind) {
    switch(kind) {
        case NProgram:
            return "Program";
        case NFun:
            return "Fun";
        case NFunParams:
            return "FunParams";
        case NFunParam:
            return "FunParam";
        case NVal:
            return "Val";
        case NType:
            return "Type";
        case NSeq:
            return "Seq";
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
        case NSym:
            return "Sym";
        case NIfElse:
            return "IfElse";
        case NIfBranch:
            return "IfBranch";
        case NElifBranch:
            return "ElifBranch";
        case NElseBranch:
            return "ElseBranch";
        case NCase:
            return "Case";
        case NCaseBranch:
            return "CaseBranch";
        case NLet:
            return "Let";
        case NMod:
            return "Mod";
        case NImport:
            return "Import";
        case NExport:
            return "Export";
    }
    return "UNDEFINED";
}

int isgroup(struct kbnode *node) {
    return node->kind == NProgram || node->kind == NFunParams || node->kind == NSeq || node->kind == NCallParams || node->kind == NIfElse || node->kind == NCase;
}
