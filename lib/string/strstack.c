#include "kobalt/strstack.h"
#include "kobalt/log.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

void kbstr_stack_new(struct kbstr_stack* str_stack) {
    kbobjpool_new(&str_stack->objpool, sizeof(char), NULL);
    kbvec_cstr_new(&str_stack->stack);
}

char* kbstr_stack_pushf(struct kbstr_stack* str_stack, char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    char* dest = kbobjpool_arralloc(&str_stack->objpool, len + 1);
    va_start(args, fmt);
    vsnprintf(dest, len + 1, fmt, args);
    va_end(args);

    kbvec_cstr_push(&str_stack->stack, dest);
#if DEBUG
    // kbilog("str_stack push %s", dest);
#endif
    return dest;
}

char* kbstr_stack_push(struct kbstr_stack* str_stack, char* str) {
    int len = strlen(str);

    char* dest = kbobjpool_arralloc(&str_stack->objpool, len + 1);
    strcpy(dest, str);
    kbvec_cstr_push(&str_stack->stack, dest);
#if DEBUG
    // kbilog("str_stack push %s", dest);
#endif
    return dest;
}



char* kbstr_stack_pop(struct kbstr_stack* str_stack) {
    char* str = kbvec_cstr_pop(&str_stack->stack);
    kbobjpool_pop(&str_stack->objpool, strlen(str) + 1);
#if DEBUG
    // kbilog("str_stack pop %s", str);
#endif
    return str;
}

char* kbstr_stack_peek(struct kbstr_stack* str_stack) {
    char* str = kbvec_cstr_get(&str_stack->stack, str_stack->stack.size - 1);
    return str;
}

void kbstr_stack_del(struct kbstr_stack* str_stack) {
    kbobjpool_del(&str_stack->objpool);
    kbvec_cstr_del(&str_stack->stack);
}
