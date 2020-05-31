#ifndef KBPARSER__H
#define KBPARSER__H

#include "kobalt/token.h"
#include "kobalt/syntax.h"
#include "kobalt/error.h"
#include "kobalt/source.h"

struct kbparser {
    struct kbtoken * tokens;
    struct kbnode * ast;
    struct kbnode * buffer;
    struct kberrvec errvec;
    struct kbsrc * src;
    int capacity;
    int numnodes;
    int cursor;
};

struct kbparser kbparser_make(struct kbtoken * tokens, struct kbsrc * src);

void kbparse(struct kbparser * parser);

void kbparser_destroy(struct kbparser * parser);

#endif
