#include "kobalt/strpool.h"
#include "kobalt/log.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

void kbstrpool_new(struct kbstrpool* strpool) {
    kbobjpool_new(&strpool->objpool, sizeof(char), NULL);
    kbvec_new(&strpool->lens, sizeof(int));
}

char* kbstrpool_push(struct kbstrpool* strpool, char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    kbvec_push(&strpool->lens, &len);

    char* dest = kbobjpool_arralloc(&strpool->objpool, len + 1);
    va_start(args, fmt);
    int r = vsnprintf(dest, len + 1, fmt, args);
    va_end(args);

    assert(r == len);
#if DEBUG
    // kbilog("strpool push %s", dest);
#endif
    return dest;
}


char* kbstrpool_pop(struct kbstrpool* strpool) {
    int len;
    kbvec_pop(&strpool->lens, &len);
    char* str = kbobjpool_arrpop(&strpool->objpool, len + 1);
#if DEBUG
    // kbilog("strpool pop %s", str);
#endif
    return str;
}

char* kbstrpool_peek(struct kbstrpool* strpool) {
    int len;
    kbvec_pop(&strpool->lens, &len);
    char* str = kbobjpool_arrpeek(&strpool->objpool, len + 1);
    return str;
}

void kbstrpool_del(struct kbstrpool* strpool) {
    kbobjpool_del(&strpool->objpool);
    kbvec_del(&strpool->lens);
}