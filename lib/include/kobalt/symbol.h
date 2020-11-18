#ifndef KBSYMBOL__H
#define KBSYMBOL__H

#include "kobalt/kobalt.h"
#include <stdbool.h>

enum kbsymbol_kind {
    NoneSym,
    FunSym,
    ValSym,
};

struct kbfunsymbol {
    int defined;
    struct kbtype* type;
};

struct kbsymbol {
    enum kbsymbol_kind kind;
    struct kbtype* type;
    union {
        struct kbfunsymbol fun;
    } data;
};

void kbsymbol_new(struct kbsymbol* symbol, enum kbsymbol_kind kind, struct kbtype* type);
void kbsymbol_del(struct kbsymbol* symbol);

#endif
