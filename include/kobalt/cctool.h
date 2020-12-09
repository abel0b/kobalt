#ifndef KLCCTOOL__H
#define KLCCTOOL__H

#include "kobalt/kobalt.h"
#include "kobalt/options.h"
#include "klbase/str.h"
#include "klbase/vec.h"

enum kl_cc {
    CCGcc = 0,
    CCClang = 1,
    CCTiny = 2,
    CCNone = 3,
};

struct kl_cmdcc {
    enum kl_cc cc;
    char* version;
};

void kl_cmdcc_new(struct kl_cmdcc* cmdcc);

int kl_cmdcc_compile(struct kl_opts* opts, struct kl_cmdcc* cmdcc, struct kl_str* src);

int kl_cmdcc_link(struct kl_opts* opts, struct kl_cmdcc* cmdcc, struct kl_vec_str* objs, struct kl_str* bin);

void kl_cmdcc_del(struct kl_cmdcc* cmdcc);

int kl_cc(struct kl_opts* opts, struct kl_str* src);

int kl_link(struct kl_opts* opts, struct kl_vec_str* objs, struct kl_str* bin);

#endif
