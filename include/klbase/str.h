#ifndef KLSTR__H
#define KLSTR__H

#include "klbase/vec.h"

struct kl_str {
    char* data;
    int len;
    int cap;
};

void kl_str_new(struct kl_str* str);
void kl_str_catf(struct kl_str* str, char* fmt, ...);
void kl_str_cat(struct kl_str* str, char* src);
void kl_str_resize(struct kl_str* str, long int len);
void kl_str_del(struct kl_str* str);

kl_vec_decl(struct kl_str, str)

#endif
