#ifndef KBOPTIONS__H
#define KBOPTIONS__H

#include "kobalt/kobalt.h"
#include <stdio.h>

enum kbstage {
    LEX,
    PARSE,
    CODEGEN,
};

struct kbopts {
    char verbosity;
    enum kbstage stage;
    char* cwd;
    int numsrcs;
    char** srcs;
    FILE* output;
    char* cachedir;
};

void kbopts_new(int argc, char* argv[], struct kbopts* opts);

void kbopts_display(struct kbopts * options);

void kbopts_del(struct kbopts * options);

#endif
