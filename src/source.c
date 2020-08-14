#include "kobalt/source.h"
#include "kobalt/memory.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void kbsrc_new(char* filename, struct kbsrc* src) {
    src->filename = kbmalloc(sizeof(char) * (strlen(filename) + 1));
    strcpy(src->filename, filename);
    FILE * file = fopen(src->filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    int base = 0;
    {
        int i = 0;
        char * filenameit = filename;
        while(*filenameit != '\0') {
            if (*filenameit == DS) base = i+1;
            ++ i;
            ++ filenameit;
        }
    }
    src->basename = kbmalloc(sizeof(char) * (strlen(filename + base) + 1));
    strcpy(src->basename, filename + base);

    fseek(file, 0, SEEK_END);
    src->length = ftell(file);
    fseek(file, 0, SEEK_SET);
    src->content = kbmalloc(src->length);
    assert(fread(src->content, src->length, 1, file) == (unsigned long)1); 
    fclose(file);
}

void kbsrc_del(struct kbsrc * src) {
    kbfree(src->content);
    kbfree(src->filename);
    kbfree(src->basename);
}
