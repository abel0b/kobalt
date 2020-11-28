#ifndef KBCCTOOL__H
#define KBCCTOOL__H

#include "kobalt/kobalt.h"
#include "kobalt/options.h"
#include "kobalt/str.h"
#include "kobalt/vec.h"

enum kbcc {
    CCGcc = 0,
    CCClang = 1,
    CCTiny = 2,
    CCNone = 3,
};

struct kbcmdcc {
    enum kbcc cc;
    char* version;
};

void kbcmdcc_new(struct kbcmdcc* cmdcc);

int kbcmdcc_compile(struct kbopts* opts, struct kbcmdcc* cmdcc, struct kbstr* src);

int kbcmdcc_link(struct kbopts* opts, struct kbcmdcc* cmdcc, struct kbvec_str* objs, struct kbstr* bin);

void kbcmdcc_del(struct kbcmdcc* cmdcc);

int kbcc(struct kbopts* opts, struct kbstr* src);

int kblink(struct kbopts* opts, struct kbvec_str* objs, struct kbstr* bin);

#endif
