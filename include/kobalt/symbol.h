#ifndef KLSYMBOL__H
#define KLSYMBOL__H

#include "kobalt/kobalt.h"
#include <stdbool.h>

enum kl_symbol_kind {
    NoneSym,
    FunSym,
    ValSym,
};

struct kl_funsymbol {
    int defined;
    struct kl_type* type;
};

struct kl_symbol {
    enum kl_symbol_kind kind;
    struct kl_type* type;
    union {
        struct kl_funsymbol fun;
    } data;
};

void kl_symbol_new(struct kl_symbol* symbol, enum kl_symbol_kind kind, struct kl_type* type);
void kl_symbol_del(struct kl_symbol* symbol);

#endif
