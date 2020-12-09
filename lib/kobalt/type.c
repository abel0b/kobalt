#include "kobalt/type.h"
#include <stdio.h>

kl_vec_impl(struct kl_type*, type)

void kl_type_new(struct kl_type* type, enum kl_typekind kind) {
    type->kind = kind;
}

void kl_type_del(struct kl_type* type) {
      switch (type->kind) {
        case Fun:
            kl_funtype_del(&type->data.fun);
            break;
        default:
            break;
      }
}

void kl_funtype_new(struct kl_funtype* funtype, struct kl_type* out_type) {
    kl_vec_type_new(&funtype->in_types);
    funtype->out_type = out_type;
}

void kl_funtype_addparam(struct kl_funtype* funtype, struct kl_type* in_type) {
    kl_vec_type_push(&funtype->in_types, in_type);
}

void kl_funtype_del(struct kl_funtype* funtype) {
    kl_vec_type_del(&funtype->in_types);
}

void kl_arraytype_new(struct kl_arraytype* arraytype, struct kl_type* elem_type) {
    arraytype->elem_type = elem_type;
}

void kl_arraytype_del(struct kl_arraytype* arraytype) {
    unused(arraytype);
}

void kl_type_display(struct kl_type* type) {
    if (type == NULL) {
        printf("NULL");
        return;
    }
    switch (type->kind) {
        case Unit:
            printf("()");
            break;
        case Int:
            printf("int");
            break;
        case Char:
            printf("char");
            break;
        case Float:
            printf("float");
            break;
        case Fun:
            printf("(");
            for(int i = 0; i < type->data.fun.in_types.size; ++i) {
                if (i != 0) {
                    printf(" ");
                }
                kl_type_display(kl_vec_type_get(&type->data.fun.in_types, i));
            }
            printf(")");
            printf(" -> ");
            kl_type_display(type->data.fun.out_type);
            break;
        case Array:
            printf("[]");
            kl_type_display(type->data.array.elem_type);
            break;
        case Str:
            printf("str");
            break;
        case Var:
            printf("'x");
            break;
    }
}
