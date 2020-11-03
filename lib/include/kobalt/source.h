#ifndef KBSOURCE__H
#define KBSOURCE__H

#include "kobalt/kobalt.h"
#include "kobalt/options.h"

struct kbsrc {
    char* filename;
    char* basename;
    long length;
    char* content;
};

void kbsrc_new(char* filename, struct kbsrc* src);

void kbsrc_del(struct kbsrc* src);

#endif
