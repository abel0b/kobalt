#ifndef KLMOD__H
#define KLMOD__H

#include "kobalt/astinfo.h"
#include "klbase/str.h"
#include "kobalt/ast.h"

struct kl_mod {
    struct kl_astinfo astinfo;
    struct kl_vec_str deps;
};

void kl_mod_new(struct kl_mod* mod, struct kl_ast* ast);

void kl_mod_del(struct kl_mod* mod);

#endif
