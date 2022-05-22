#ifndef KLOPTIONS__H
#define KLOPTIONS__H

#include "kobalt/kobalt.h"
#include "abl/vec.h"
#include "abl/queue.h"
#include "kobalt/compiland.h"
#include "abl/str.h"
#include <stdio.h>
#include <stdbool.h>
#include "kobalt/stage.h"

struct kl_opts {
    int optim;
    int verbosity;
    int stages;
    bool color;
    struct abl_str cwd;
    struct abl_vec inputs;
    struct abl_str outpath;
    struct abl_str cachepath;
    struct abl_vec exe_argv;
    struct abl_str manifest_path;
    FILE* manifest;
};

void kl_opts_new(struct kl_opts* opts, int argc, char* argv[]);

void kl_opts_del(struct kl_opts* opts);

#endif
