#ifndef ABLSTRSTACK__H
#define ABLSTRSTACK__H

#include "abl/objpool.h"
#include "abl/vec.h"
#include "abl/defs.h"

struct abl_str_stack {
    struct abl_objpool objpool;
    struct abl_vec_cstr stack;
};

ABL_API void abl_str_stack_new(struct abl_str_stack* strstack);

ABL_API char* abl_str_stack_push(struct abl_str_stack* str_stack, char* str);

ABL_API char* abl_str_stack_pushf(struct abl_str_stack* strstack, char * fmt, ...);

ABL_API char* abl_str_stack_pop(struct abl_str_stack* strstack);

ABL_API char* abl_str_stack_peek(struct abl_str_stack* strstack);

ABL_API void abl_str_stack_del(struct abl_str_stack* strstack);

#endif
