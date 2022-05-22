#ifndef KLCCTOOL__H
#define KLCCTOOL__H

#include "kobalt/kobalt.h"
#include "kobalt/options.h"
#include "abl/str.h"
#include "abl/vec.h"

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

int kl_cmdcc_compile(struct kl_opts* opts, struct kl_cmdcc* cmdcc, struct abl_str* src);

int kl_cmdcc_link(struct kl_opts* opts, struct kl_cmdcc* cmdcc, struct abl_vec_str* objs, struct abl_str* bin);

void kl_cmdcc_del(struct kl_cmdcc* cmdcc);

int kl_cc(struct kl_opts* opts, struct abl_str* src);

int kl_link(struct kl_opts* opts, struct abl_vec_str* objs, struct abl_str* bin);

#endif
