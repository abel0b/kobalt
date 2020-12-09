#include "klbase/strstack.h"
#include "klbase/log.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

void kl_str_stack_new(struct kl_str_stack* str_stack) {
    kl_objpool_new(&str_stack->objpool, sizeof(char), NULL);
    kl_vec_cstr_new(&str_stack->stack);
}

char* kl_str_stack_pushf(struct kl_str_stack* str_stack, char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    char* dest = kl_objpool_arralloc(&str_stack->objpool, len + 1);
    va_start(args, fmt);
    vsnprintf(dest, len + 1, fmt, args);
    va_end(args);

    kl_vec_cstr_push(&str_stack->stack, dest);
#if DEBUG
    // kl_ilog("str_stack push %s", dest);
#endif
    return dest;
}

char* kl_str_stack_push(struct kl_str_stack* str_stack, char* str) {
    int len = strlen(str);

    char* dest = kl_objpool_arralloc(&str_stack->objpool, len + 1);
    strcpy(dest, str);
    kl_vec_cstr_push(&str_stack->stack, dest);
#if DEBUG
    // kl_ilog("str_stack push %s", dest);
#endif
    return dest;
}



char* kl_str_stack_pop(struct kl_str_stack* str_stack) {
    char* str = kl_vec_cstr_pop(&str_stack->stack);
    kl_objpool_pop(&str_stack->objpool, strlen(str) + 1);
    return str;
}

char* kl_str_stack_peek(struct kl_str_stack* str_stack) {
    char* str = kl_vec_cstr_get(&str_stack->stack, str_stack->stack.size - 1);
    return str;
}

void kl_str_stack_del(struct kl_str_stack* str_stack) {
    kl_objpool_del(&str_stack->objpool);
    kl_vec_cstr_del(&str_stack->stack);
}
