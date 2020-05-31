#ifndef KBOPTIONS__H
#define KBOPTIONS__H

#include "kobalt/kobalt.h"
#include <stdio.h>

enum kbstage {
    LEX,
    PARSE,
};

struct kbopts {
    char verbosity;
    enum kbstage stage;
    char * cwd;
    unsigned int num_srcs;
    struct kbsrc * srcs;
    FILE * output;
};

struct kbopts kbopts_make(int argc, char * argv[]);

void kbopts_display(struct kbopts * options);

void kbopts_destroy(struct kbopts * options);

#endif
