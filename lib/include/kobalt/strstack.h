#ifndef KBSTRSTACK__H
#define KBSTRSTACK__H

#include "kobalt/kobalt.h"
#include "kobalt/objpool.h"
#include "kobalt/vec.h"

struct kbstr_stack {
    struct kbobjpool objpool;
    struct kbvec_cstr stack;
};

void kbstr_stack_new(struct kbstr_stack* strstack);

char* kbstr_stack_push(struct kbstr_stack* str_stack, char* str);

char* kbstr_stack_pushf(struct kbstr_stack* strstack, char * fmt, ...);

char* kbstr_stack_pop(struct kbstr_stack* strstack);

char* kbstr_stack_peek(struct kbstr_stack* strstack);

void kbstr_stack_del(struct kbstr_stack* strstack);

#endif
