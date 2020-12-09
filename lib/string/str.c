#include "klbase/str.h"
#include "klbase/log.h"
#include "klbase/mem.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

kl_vec_impl(struct kl_str, str)

void kl_str_new(struct kl_str* str) {
    str->len = 0;
    str->cap = 8;
    str->data = kl_malloc(sizeof(str->data[0]) * str->cap);
    str->data[0] = '\0';
}

void kl_str_catf(struct kl_str* str, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    while(str->cap < str->len + len + 1) {
        str->cap = (str->cap)? 2 * str->cap : 8;
        str->data = kl_realloc(str->data, sizeof(str->data[0]) * str->cap);
    }

    va_start(args, fmt);
    vsnprintf(&str->data[str->len], len + 1, fmt, args);
    va_end(args);
    str->len += len;
}

void kl_str_cat(struct kl_str* str, char* src) {
    int len = strlen(src);
    while(str->cap < str->len + len + 1) {
        str->cap = (str->cap)? 2 * str->cap : 8;
        str->data = kl_realloc(str->data, sizeof(str->data[0]) * str->cap);
    }
    strcpy(&str->data[str->len], src);
    str->len += len;
}

void kl_str_resize(struct kl_str* str, long int len) {
    if (len + 1 > str->cap) {
        str->cap = len + 1;
        str->data = kl_realloc(str->data, sizeof(str->data[0]) * str->cap);
    }
    str->len = len;
    str->data[str->len] = '\0';
}

void kl_str_del(struct kl_str* str) {
    if (str->data) {
        kl_free(str->data);
    }
}
