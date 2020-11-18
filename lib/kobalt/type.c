#include "kobalt/type.h"
#include <stdio.h>

kbvec_impl(struct kbtype*, type)

void kbtype_new(struct kbtype* type, enum kbtypekind kind) {
    type->kind = kind;
}

void kbtype_del(struct kbtype* type) {
      switch (type->kind) {
        case Fun:
            kbfuntype_del(&type->data.fun);
            break;
        default:
            break;
      }
}

void kbfuntype_new(struct kbfuntype* funtype, struct kbtype* out_type) {
    kbvec_type_new(&funtype->in_types);
    funtype->out_type = out_type;
}

void kbfuntype_addparam(struct kbfuntype* funtype, struct kbtype* in_type) {
    kbvec_type_push(&funtype->in_types, in_type);
}

void kbfuntype_del(struct kbfuntype* funtype) {
    kbvec_type_del(&funtype->in_types);
}

void kbarraytype_new(struct kbarraytype* arraytype, struct kbtype* elem_type) {
    arraytype->elem_type = elem_type;
}

void kbarraytype_del(struct kbarraytype* arraytype) {
    unused(arraytype);
}

void kbtype_display(struct kbtype* type) {
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
                kbtype_display(kbvec_type_get(&type->data.fun.in_types, i));
            }
            printf(")");
            printf(" -> ");
            kbtype_display(type->data.fun.out_type);
            break;
        case Array:
            printf("[]");
            kbtype_display(type->data.array.elem_type);
            break;
        case Str:
            printf("str");
            break;
        case Var:
            printf("'x");
            break;
    }
}
