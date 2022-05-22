#include "kobalt/mod.h"

void kl_mod_new(struct kl_mod* mod, struct kl_ast* ast) {
    kl_astinfo_new(&mod->astinfo, ast);
    abl_vec_str_new(&mod->deps);
}

void kl_mod_del(struct kl_mod* mod) {
    kl_astinfo_del(&mod->astinfo);
    for(int i = 0; i < mod->deps.size; ++i) {
        abl_str_del(&mod->deps.data[i]);
    }
    abl_vec_str_del(&mod->deps);
}
