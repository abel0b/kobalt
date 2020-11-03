#ifndef KBSTR__H
#define KBSTR__H

struct kbstr {
    char* data;
    int len;
    int cap;
};

void kbstr_new(struct kbstr* str);
void kbstr_catf(struct kbstr* str, char* fmt, ...);
void kbstr_del(struct kbstr* str);

#endif
