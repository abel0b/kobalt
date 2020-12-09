#ifndef KLTYPE__H
#define KLTYPE__H

#include "kobalt/kobalt.h"
#include "klbase/vec.h"

kl_vec_decl(struct kl_type*, type)

enum kl_typekind {
    Unit = 0,
    Int = 1,
    Char = 2,
    Float = 3,
    Fun = 4,
    Array = 5,
    Var = 6,
    Str = 7,
};

struct kl_arraytype {
    struct kl_type* elem_type;
};

struct kl_funtype {
    struct kl_vec_type in_types;
    struct kl_type* out_type;
};

struct kl_type {
    enum kl_typekind kind;
    union {
        struct kl_funtype fun;
        struct kl_arraytype array;
    } data;
};

void kl_type_new(struct kl_type* type, enum kl_typekind kind);
void kl_type_del(struct kl_type* type);
void kl_type_display(struct kl_type* type);

void kl_funtype_new(struct kl_funtype* funtype, struct kl_type* out_type);
void kl_funtype_addparam(struct kl_funtype* funtype, struct kl_type* in_type);
void kl_funtype_del(struct kl_funtype* funtype);

void kl_arraytype_new(struct kl_arraytype* arraytype, struct kl_type* elem_type);
void kl_arraytype_del(struct kl_arraytype* arraytype);

#endif
