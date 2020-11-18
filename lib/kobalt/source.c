#include "kobalt/source.h"
#include "kobalt/memory.h"
#include "kobalt/log.h"
#include "kobalt/fs.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "kobalt/stdsrc.h"

void kbsrc_new(char* filename, struct kbsrc* src) {
    src->filename = kbmalloc(sizeof(char) * (strlen(filename) + 1));
    strcpy(src->filename, filename);
    FILE * file = fopen(src->filename, "rb");
    if (file == NULL) {
        perror("fopen");
        kbelog("could not open source file '%s'", filename);
        exit(1);
    }

    int base = 0;
    {
        int i = 0;
        char * filenameit = filename;
        while(*filenameit != '\0') {
            if (isds(*filenameit)){
                base = i+1;
            }
            ++ i;
            ++ filenameit;
        }
    }

    src->basename = kbmalloc(sizeof(char) * (strlen(filename + base) + 1));
    strcpy(src->basename, filename + base);

    fseek(file, 0, SEEK_END);
    int srclen = ftell(file);
    fseek(file, 0, SEEK_SET);

    src->length = srclen + sizeof(stdkb) + 1 + 1;
    src->content = kbmalloc(src->length+1000);

    memcpy(src->content, stdkb, sizeof(stdkb));
    src->content[sizeof(stdkb)] = '\n';
    assert(fread(src->content + sizeof(stdkb) + 1, srclen, 1, file) == (unsigned long)1);
    src->content[src->length - 1] = '0';

    // printf("%s",src->content);
    fclose(file);
}

void kbsrc_del(struct kbsrc * src) {
    kbfree(src->content);
    kbfree(src->filename);
    kbfree(src->basename);
}
