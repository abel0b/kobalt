#include "abl/str.h"
#include "abl/log.h"
#include "abl/mem.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

abl_vec_impl(struct abl_str, str)

ABL_API void abl_str_new(struct abl_str* str) {
    str->len = 0;
    str->cap = 8;
    str->data = abl_malloc(sizeof(str->data[0]) * str->cap);
    str->data[0] = '\0';
}

ABL_API void abl_str_catf(struct abl_str* str, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    while(str->cap < str->len + len + 1) {
        str->cap = (str->cap)? 2 * str->cap : 8;
        str->data = abl_realloc(str->data, sizeof(str->data[0]) * str->cap);
    }

    va_start(args, fmt);
    vsnprintf(&str->data[str->len], len + 1, fmt, args);
    va_end(args);
    str->len += len;
}

ABL_API void abl_str_cat(struct abl_str* str, char* src) {
    int len = strlen(src);
    while(str->cap < str->len + len + 1) {
        str->cap = (str->cap)? 2 * str->cap : 8;
        str->data = abl_realloc(str->data, sizeof(str->data[0]) * str->cap);
    }
    strcpy(&str->data[str->len], src);
    str->len += len;
}

ABL_API void abl_str_resize(struct abl_str* str, long int len) {
    if (len + 1 > str->cap) {
        str->cap = len + 1;
        str->data = abl_realloc(str->data, sizeof(str->data[0]) * str->cap);
    }
    str->len = len;
    str->data[str->len] = '\0';
}

ABL_API void abl_str_del(struct abl_str* str) {
    if (str->data) {
        abl_free(str->data);
    }
}
