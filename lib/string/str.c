#include "kobalt/str.h"
#include "kobalt/log.h"
#include "kobalt/memory.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

void kbstr_new(struct kbstr* str) {
    str->data = NULL;
    str->len = 0;
    str->cap = 0;
}

void kbstr_catf(struct kbstr* str, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    while(str->cap < str->len + len + 1) {
        str->cap = (str->cap)? 2 * str->cap : 8;
        str->data = kbrealloc(str->data, sizeof(str->data[0]) * str->cap);
    }

    va_start(args, fmt);
    vsnprintf(&str->data[str->len], len + 1, fmt, args);
    va_end(args);
    str->len += len;
}

void kbstr_del(struct kbstr* str) {
    if (str->data) {
        kbfree(str->data);
    }
}
