#include "kobalt/compiland.h"
#include "abl/mem.h"
#include "abl/log.h"
#include "abl/fs.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

static void kl_compiland_new_aux(struct kl_compiland* compiland, char* filename, bool entry) {
    compiland->virtual = false;
    compiland->entry = entry;
    compiland->boilerplate = true;
    abl_str_new(&compiland->path);
    abl_str_cat(&compiland->path, filename);
    abl_path_normalize(&compiland->path);

    int base = 0;
    {
        int i = 0;
        char * filenameit = filename;
        while(*filenameit != '\0') {
            if (abl_path_isds(*filenameit)){
                base = i+1;
            }
            ++ i;
            ++ filenameit;
        }
    }

    abl_str_new(&compiland->basename);
    abl_str_cat(&compiland->basename, filename + base);
    abl_str_resize(&compiland->basename, compiland->basename.len - 3);

    abl_str_new(&compiland->name);
    for(int i = 0; i < compiland->path.len - 3; ++ i) {
        char c = (abl_path_isds(compiland->path.data[i]))? '%' : compiland->path.data[i];
        if (c != ':') {
            abl_str_catf(&compiland->name, "%c", c);
        }
    }

    FILE * file = fopen(compiland->path.data, "rb");
    if (file == NULL) {
        perror("fopen");
        abl_elog("could not open source file '%s'", compiland->path.data);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long int filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    abl_str_new(&compiland->content);

    if (filesize) {
        abl_str_resize(&compiland->content, filesize);
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

    abl_str_new(&compiland->path);
    abl_str_cat(&compiland->path, path);
    abl_path_normalize(&compiland->path);

    int base = 0;
    {
        int i = 0;
        char * filenameit = path;
        while(*filenameit != '\0') {
            if (abl_path_isds(*filenameit)){
                base = i+1;
            }
            ++ i;
            ++ filenameit;
        }
    }

    abl_str_new(&compiland->basename);
    abl_str_cat(&compiland->basename, path + base);
    abl_str_resize(&compiland->basename, compiland->basename.len - 3);

    abl_str_new(&compiland->name);
    for(int i = 0; i < compiland->path.len - 3; ++ i) {
        char c = (abl_path_isds(compiland->path.data[i]))? '%' : compiland->path.data[i];
        abl_str_catf(&compiland->name, "%c", c);
    }

    abl_str_new(&compiland->content);
    abl_str_cat(&compiland->content, content);
}

void kl_compiland_del(struct kl_compiland * compiland) {
    abl_str_del(&compiland->path);
    abl_str_del(&compiland->basename);
    abl_str_del(&compiland->name);
    abl_str_del(&compiland->content);
}
