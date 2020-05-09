#include "kobalt/source.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct kbsrc kbsrc_make(struct kbopts * options, char * filename) {
    (void)options;
    struct kbsrc src;
    src.filename = malloc(sizeof(char) * (strlen(filename) + 1));
    strcpy(src.filename, filename);
    FILE * file = fopen(src.filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    src.length = ftell(file);
    fseek(file, 0, SEEK_SET);
    src.content = malloc(src.length);
    assert(fread(src.content, src.length, 1, file) == (unsigned long)1); 
    fclose(file);
    return src;
}

void kbsrc_destroy(struct kbsrc * src) {
    free(src->content);
    free(src->filename);
}
