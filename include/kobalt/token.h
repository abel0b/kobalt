#ifndef KBTOKEN__H
#define KBTOKEN__H

enum kbtoken_kind {
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_STRLIT,
    TOKEN_INDENT,
    TOKEN_DEDENT,
    TOKEN_LF,
    TOKEN_LPAR,
    TOKEN_RPAR,
    TOKEN_COLON,
    TOKEN_EQUAL,
    TOKEN_KEYWORD
};

struct kbtoken {
    enum kbtoken_kind kind;
    char * value;
    int size;
};

struct kbtoken kbtoken_make(enum kbtoken_kind kind, char * value);

char * kbtoken_string(enum kbtoken_kind type);

void kbtoken_debug(struct kbtoken * token);

void kbtoken_destroy(struct kbtoken * token);

#endif
