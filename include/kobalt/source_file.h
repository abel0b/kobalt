#ifndef KOBALT__SOURCE_FILE__H
#define KOBALT__SOURCE_FILE__H

#include "kobalt/options.h"

struct SourceFile {
    char * path;
    char * filename;
    char * content;
    F
};

struct SourceFile * source_file_read(struct KobaltOptions * options, char * filename);

void source_file_destroy(struct SourceFile * source_file);

#endif
