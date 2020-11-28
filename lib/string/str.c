#include "kobalt/str.h"
#include "kobalt/log.h"
#include "kobalt/memory.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

kbvec_impl(struct kbstr, str)

void kbstr_new(struct kbstr* str) {
    str->len = 0;
    str->cap = 8;
    str->data = kbmalloc(sizeof(str->data[0]) * str->cap);
    str->data[0] = '\0';
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

void kbstr_cat(struct kbstr* str, char* src) {
    int len = strlen(src);
    while(str->cap < str->len + len + 1) {
        str->cap = (str->cap)? 2 * str->cap : 8;
        str->data = kbrealloc(str->data, sizeof(str->data[0]) * str->cap);
    }
    strcpy(&str->data[str->len], src);
    str->len += len;
}

void kbstr_resize(struct kbstr* str, long int len) {
    if (len + 1 > str->cap) {
        str->cap = len + 1;
        str->data = kbrealloc(str->data, sizeof(str->data[0]) * str->cap);
    }
    str->len = len;
    str->data[str->len] = '\0';
}

void kbstr_del(struct kbstr* str) {
    if (str->data) {
        kbfree(str->data);
    }
}
