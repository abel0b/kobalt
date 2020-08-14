#ifndef KBPARSER__H
#define KBPARSER__H

#include "kobalt/kobalt.h"
#include "kobalt/token.h"
#include "kobalt/syntax.h"
#include "kobalt/error.h"
#include "kobalt/source.h"
#include "kobalt/ast.h"

struct kbparser {
    struct kbtoken * tokens;
    int curnode;
    struct kbast * ast;
    struct kberrvec errvec;
    struct kbsrc * src;
    int capacity;
    int numnodes;
    int cursor;
};

struct kbparser kbparser_make(struct kbtoken * tokens, struct kbsrc * src);

void kbparser_new(struct kbparser* parser, struct kbtoken* tokens, struct kbsrc* src, struct kbast* ast);

void kbparser_run(struct kbparser* parser);

void kbparser_del(struct kbparser * parser);

void kbparse(struct kbtoken* tokens, struct kbsrc* src, struct kbast* ast);

#endif
