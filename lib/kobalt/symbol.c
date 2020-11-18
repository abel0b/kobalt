#include "kobalt/symbol.h"

void kbsymbol_new(struct kbsymbol* symbol, enum kbsymbol_kind kind, struct kbtype* type) {
    symbol->kind = kind;
    symbol->type = type;
    switch(kind) {
        case FunSym:
            symbol->data.fun.defined = false;
            break;
        case ValSym:
            break;
        case NoneSym:
            break;
        default:
            kb_todo("handle symbol kind case %d", kind);
            break;
    }
}

void kbsymbol_del(struct kbsymbol* symbol) {
    kb_unused(symbol);
}
