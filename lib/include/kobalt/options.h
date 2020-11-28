#ifndef KBOPTIONS__H
#define KBOPTIONS__H

#include "kobalt/kobalt.h"
#include "kobalt/vec.h"
#include "kobalt/queue.h"
#include "kobalt/compiland.h"
#include "kobalt/str.h"
#include <stdio.h>
#include <stdbool.h>
#include "kobalt/stage.h"

struct kbopts {
    int optim;
    int verbosity;
    int stages;
    struct kbstr cwd;
    struct kbvec inputs;
    struct kbstr outpath;
    struct kbstr cachepath;
    struct kbvec exe_argv;
};

void kbopts_new(struct kbopts* opts, int argc, char* argv[]);

void kbopts_del(struct kbopts* opts);

#endif
