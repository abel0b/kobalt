#include "abl/strstack.h"
#include "abl/log.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

ABL_API void abl_str_stack_new(struct abl_str_stack* str_stack) {
    abl_objpool_new(&str_stack->objpool, sizeof(char), NULL);
    abl_vec_cstr_new(&str_stack->stack);
}

ABL_API char* abl_str_stack_pushf(struct abl_str_stack* str_stack, char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    char* dest = abl_objpool_arralloc(&str_stack->objpool, len + 1);
    va_start(args, fmt);
    vsnprintf(dest, len + 1, fmt, args);
    va_end(args);

    abl_vec_cstr_push(&str_stack->stack, dest);
#if DEBUG
    // abl_ilog("str_stack push %s", dest);
#endif
    return dest;
}

ABL_API char* abl_str_stack_push(struct abl_str_stack* str_stack, char* str) {
    int len = strlen(str);

    char* dest = abl_objpool_arralloc(&str_stack->objpool, len + 1);
    strcpy(dest, str);
    abl_vec_cstr_push(&str_stack->stack, dest);
#if DEBUG
    // abl_ilog("str_stack push %s", dest);
#endif
    return dest;
}

ABL_API char* abl_str_stack_pop(struct abl_str_stack* str_stack) {
    char* str = abl_vec_cstr_pop(&str_stack->stack);
    abl_objpool_pop(&str_stack->objpool, strlen(str) + 1);
    return str;
}

ABL_API char* abl_str_stack_peek(struct abl_str_stack* str_stack) {
    char* str = abl_vec_cstr_get(&str_stack->stack, str_stack->stack.size - 1);
    return str;
}

ABL_API void abl_str_stack_del(struct abl_str_stack* str_stack) {
    abl_objpool_del(&str_stack->objpool);
    abl_vec_cstr_del(&str_stack->stack);
}
