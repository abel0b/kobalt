#ifndef KBTYPE__H
#define KBTYPE__H

#include "kobalt/kobalt.h"
#include "kobalt/vec.h"

kbvec_decl(struct kbtype*, type)

enum kbtypekind {
    Unit = 0,
    Int = 1,
    Char = 2,
    Float = 3,
    Fun = 4,
    Array = 5,
    Var = 6,
    Str = 7,
};

struct kbarraytype {
    struct kbtype* elem_type;
};

struct kbfuntype {
    struct kbvec_type in_types;
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
