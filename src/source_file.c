#include "kobalt/source_file.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct SourceFile * source_file_read(struct KobaltOptions * options, char * filename) {
    struct SourceFile * source_file = malloc (sizeof (struct SourceFile));
    printf("%s %s\n", options->cwd, filename);
    // source_file->path = malloc (sizeof (char) * (strlen (options->cwd) + strlen (filename) + 2));
    // source_file->filename = malloc (sizeof (char) * (strlen (filename) + 1));
    // strcpy(source_file->filename, filename);
    // strcpy(source_file->path, options->cwd);
    // strcat(source_file->path, "/");
    // strcat(source_file->path, filename);
    // printf("%s", source_file->path);
    return source_file;
}

void source_file_destroy(struct SourceFile * source_file) {
    // free (source_file->path);
    // free (source_file->filename);
    free (source_file);
}
