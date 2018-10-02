#ifndef KOBALT__OPTIONS__H
#define KOBALT__OPTIONS__H

#include "array/array.h"

struct KobaltOptions {
    struct Array * source_files;
    char * cwd;
};

struct KobaltOptions * kobalt_options_make(int argc, char * argv[]);

void  parse_argv(struct KobaltOptions * options, int argc, char * argv[]);

void kobalt_options_destroy(struct KobaltOptions * options);

#endif
