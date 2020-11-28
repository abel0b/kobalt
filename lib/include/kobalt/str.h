#ifndef KBSTR__H
#define KBSTR__H

#include "kobalt/vec.h"

struct kbstr {
    char* data;
    int len;
    int cap;
};

void kbstr_new(struct kbstr* str);
void kbstr_catf(struct kbstr* str, char* fmt, ...);
void kbstr_cat(struct kbstr* str, char* src);
void kbstr_resize(struct kbstr* str, long int len);
void kbstr_del(struct kbstr* str);

kbvec_decl(struct kbstr, str)

#endif
