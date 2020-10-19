#ifndef KBTYPE__H
#define KBTYPE__H

#include "kobalt/kobalt.h"
#include "kobalt/vec.h"

enum kbtypekind {
    Unit = 1,
    Int = 2,
    Char = 4,
    FLoat = 8,
    Fun = 16,
    Array = 32,
    Var = 64,
};

struct kbarraytype {
    struct kbtype* elem_type;
};

struct kbfuntype {
    struct kbvec in_types;
    struct kbtype* out_type;
};

struct kbtype {
    enum kbtypekind kind;
    union {
        struct kbfuntype fun;
        struct kbarraytype array;
    } data;
};

void kbtype_new(struct kbtype* type, enum kbtypekind kind);
void kbtype_del(struct kbtype* type);
void kbtype_display(struct kbtype* type);

void kbfuntype_new(struct kbfuntype* funtype, struct kbtype* out_type);
void kbfuntype_addparam(struct kbfuntype* funtype, struct kbtype* in_type);
void kbfuntype_del(struct kbfuntype* funtype);

void kbarraytype_new(struct kbarraytype* arraytype, struct kbtype* elem_type);
void kbarraytype_del(struct kbarraytype* arraytype);

#endif
