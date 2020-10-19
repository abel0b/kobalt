#include "kobalt/type.h"
#include <stdio.h>

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
    kbvec_new(&funtype->in_types, sizeof(out_type));
    funtype->out_type = out_type;
}

void kbfuntype_addparam(struct kbfuntype* funtype, struct kbtype* in_type) {
    kbvec_push(&funtype->in_types, &in_type);
}

void kbfuntype_del(struct kbfuntype* funtype) {
    // for(int i = 0; i < funtype->in_types.num_elems; ++i) {
    //     struct kbtype* in_type = * (struct kbtype**) kbvec_get(&funtype->in_types, i);
    //     kbtype_del(in_type);
    // }
    // kbtype_del(funtype->out_type);
    kbvec_del(&funtype->in_types);
}

void kbarraytype_new(struct kbarraytype* arraytype, struct kbtype* elem_type) {
    arraytype->elem_type = elem_type;
}

void kbarraytype_del(struct kbarraytype* arraytype) {
    unused(arraytype);
    // kbtype_del(arraytype->elem_type);
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
        case FLoat:
            printf("float");
            break;
        case Fun:
            for(int i = 0; i < type->data.fun.in_types.num_elems; ++i) {
                if (i != 0) {
                    printf(" ");
                }
                kbtype_display(* (struct kbtype**) kbvec_get(&type->data.fun.in_types, i));
            }
            printf(" -> ");
            kbtype_display(type->data.fun.out_type);
            break;
        case Array:
            printf("[]");
            kbtype_display(type->data.array.elem_type);
            break;
        case Var:
            printf("'x");
            break;
    }
}
