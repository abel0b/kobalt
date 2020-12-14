#ifndef KLOPTIONS__H
#define KLOPTIONS__H

#include "kobalt/kobalt.h"
#include "klbase/vec.h"
#include "klbase/queue.h"
#include "kobalt/compiland.h"
#include "klbase/str.h"
#include <stdio.h>
#include <stdbool.h>
#include "kobalt/stage.h"

struct kl_opts {
    int optim;
    int verbosity;
    int stages;
    bool color;
    struct kl_str cwd;
    struct kl_vec inputs;
    struct kl_str outpath;
    struct kl_str cachepath;
    struct kl_vec exe_argv;
    struct kl_str manifest_path;
    FILE* manifest;
};

void kl_opts_new(struct kl_opts* opts, int argc, char* argv[]);

void kl_opts_del(struct kl_opts* opts);

#endif
