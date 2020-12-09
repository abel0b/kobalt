#include "kobalt/compiland.h"
#include "klbase/mem.h"
#include "klbase/log.h"
#include "klbase/fs.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

static void kl_compiland_new_aux(struct kl_compiland* compiland, char* filename, bool entry) {
    compiland->virtual = false;
    compiland->entry = entry;
    compiland->boilerplate = true;
    kl_str_new(&compiland->path);
    kl_str_cat(&compiland->path, filename);
    kl_path_normalize(&compiland->path);

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

    kl_str_new(&compiland->basename);
    kl_str_cat(&compiland->basename, filename + base);
    kl_str_resize(&compiland->basename, compiland->basename.len - 3);

    kl_str_new(&compiland->name);
    for(int i = 0; i < compiland->path.len - 3; ++ i) {
        char c = (isds(compiland->path.data[i]))? '%' : compiland->path.data[i];
        if (c != ':') {
            kl_str_catf(&compiland->name, "%c", c);
        }
    }

    FILE * file = fopen(compiland->path.data, "rb");
    if (file == NULL) {
        perror("fopen");
        kl_elog("could not open source file '%s'", compiland->path.data);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long int filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    kl_str_new(&compiland->content);

    if (filesize) {
        kl_str_resize(&compiland->content, filesize);
        size_t r = fread(compiland->content.data, filesize, 1, file);
        assert(r == 1);
    }
   
    fclose(file);
}

void kl_compiland_new_entry(struct kl_compiland* compiland, char* path) {
    kl_compiland_new_aux(compiland, path, true);
}

void kl_compiland_new(struct kl_compiland* compiland, char* path) {
    kl_compiland_new_aux(compiland, path, false);
}

void kl_compiland_new_virt(struct kl_compiland* compiland, char* path, char* content) {
    compiland->virtual = true;
    compiland->entry = false;
    compiland->boilerplate = false;

    kl_str_new(&compiland->path);
    kl_str_cat(&compiland->path, path);
    kl_path_normalize(&compiland->path);

    int base = 0;
    {
        int i = 0;
        char * filenameit = path;
        while(*filenameit != '\0') {
            if (isds(*filenameit)){
                base = i+1;
            }
            ++ i;
            ++ filenameit;
        }
    }

    kl_str_new(&compiland->basename);
    kl_str_cat(&compiland->basename, path + base);
    kl_str_resize(&compiland->basename, compiland->basename.len - 3);

    kl_str_new(&compiland->name);
    for(int i = 0; i < compiland->path.len - 3; ++ i) {
        char c = (isds(compiland->path.data[i]))? '%' : compiland->path.data[i];
        kl_str_catf(&compiland->name, "%c", c);
    }

    kl_str_new(&compiland->content);
    kl_str_cat(&compiland->content, content);
}

void kl_compiland_del(struct kl_compiland * compiland) {
    kl_str_del(&compiland->path);
    kl_str_del(&compiland->basename);
    kl_str_del(&compiland->name);
    kl_str_del(&compiland->content);
}
