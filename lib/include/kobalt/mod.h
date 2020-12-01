#ifndef KBMOD__H
#define KBMOD__H

#include "kobalt/astinfo.h"
#include "kobalt/str.h"
#include "kobalt/ast.h"

struct kbmod {
    struct kbastinfo astinfo;
    struct kbvec_str deps;
};

void kbmod_new(struct kbmod* mod, struct kbast* ast);

void kbmod_del(struct kbmod* mod);

#endif
