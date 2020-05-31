#ifndef KBSOURCE__H
#define KBSOURCE__H

#include "kobalt/options.h"

struct kbsrc {
    char * filename;
    long length;
    char * content;
};

struct kbsrc kbsrc_make(struct kbopts * options, char * filename);

void kbsrc_destroy(struct kbsrc * src);

#endif
