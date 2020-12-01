#include "kobalt/mod.h"

void kbmod_new(struct kbmod* mod, struct kbast* ast) {
    kbastinfo_new(&mod->astinfo, ast);
    kbvec_str_new(&mod->deps);
}

void kbmod_del(struct kbmod* mod) {
    kbastinfo_del(&mod->astinfo);
    for(int i = 0; i < mod->deps.size; ++i) {
        kbstr_del(&mod->deps.data[i]);
    }
    kbvec_str_del(&mod->deps);
}
