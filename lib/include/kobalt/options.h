#ifndef KBOPTIONS__H
#define KBOPTIONS__H

#include "kobalt/kobalt.h"
#include "kobalt/vec.h"
#include <stdio.h>

enum kbstage {
    LexingStage = 1,
    ParsingStage = 2,
    TypingStage = 4,
    CGenStage = 8,
    CCStage = 16,
    ExecStage = 32,
};

struct kbopts {
    int optim;
    int run;
    char verbosity;
    enum kbstage stage;
    char* cwd;
    int numsrcs;
    char** srcs;
    char* output;
    char* cachedir;
    struct kbvec exe_argv;
};

void kbopts_new(int argc, char* argv[], struct kbopts* opts);

void kbopts_del(struct kbopts * options);

#endif
