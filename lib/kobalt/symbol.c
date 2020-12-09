#include "kobalt/symbol.h"

void kl_symbol_new(struct kl_symbol* symbol, enum kl_symbol_kind kind, struct kl_type* type) {
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
            kl_todo("handle symbol kind case %d", kind);
            break;
    }
}

void kl_symbol_del(struct kl_symbol* symbol) {
    kl_unused(symbol);
}
