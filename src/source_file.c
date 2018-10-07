#include "kobalt/source_file.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct SourceFile * source_file_read(struct KobaltOptions * options, char * filename) {
    struct SourceFile * source_file = malloc (sizeof (struct SourceFile));
    source_file->path = malloc (sizeof (char) * (strlen (options->cwd) + strlen (filename) + 2));
    source_file->filename = malloc (sizeof (char) * (strlen (filename) + 1));
    strcpy(source_file->filename, filename);
    strcpy(source_file->path, options->cwd);
    strcat(source_file->path, "/");
    strcat(source_file->path, filename);
    FILE * file = fopen (source_file->path, "rb");
    fseek (file, 0, SEEK_END);
    source_file->length = ftell (file);
    fseek (file, 0, SEEK_SET);
    source_file->content = malloc (source_file->length);
    fread (source_file->content, 1, source_file->length, file);
    fclose (file);
    return source_file;
}

void source_file_destroy(struct SourceFile * source_file) {
    free (source_file->path);
    free (source_file->content);
    free (source_file->filename);
    free (source_file);
}
