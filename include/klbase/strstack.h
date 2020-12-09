#ifndef KLSTRSTACK__H
#define KLSTRSTACK__H

#include "kobalt/kobalt.h"
#include "klbase/objpool.h"
#include "klbase/vec.h"

struct kl_str_stack {
    struct kl_objpool objpool;
    struct kl_vec_cstr stack;
};

void kl_str_stack_new(struct kl_str_stack* strstack);

char* kl_str_stack_push(struct kl_str_stack* str_stack, char* str);

char* kl_str_stack_pushf(struct kl_str_stack* strstack, char * fmt, ...);

char* kl_str_stack_pop(struct kl_str_stack* strstack);

char* kl_str_stack_peek(struct kl_str_stack* strstack);

void kl_str_stack_del(struct kl_str_stack* strstack);

#endif
