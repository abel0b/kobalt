#include "kobalt/compiland.h"
#include "kobalt/memory.h"
#include "kobalt/log.h"
#include "kobalt/fs.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

static void kbcompiland_new_aux(struct kbcompiland* compiland, char* filename, bool entry) {
    compiland->virtual = false;
    compiland->entry = entry;
    compiland->boilerplate = true;
    kbstr_new(&compiland->path);
    kbstr_cat(&compiland->path, filename);
    kbpath_normalize(&compiland->path);

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

    kbstr_new(&compiland->basename);
    kbstr_cat(&compiland->basename, filename + base);
    kbstr_resize(&compiland->basename, compiland->basename.len - 3);

    kbstr_new(&compiland->name);
    for(int i = 0; i < compiland->path.len - 3; ++ i) {
        char c = (isds(compiland->path.data[i]))? '%' : compiland->path.data[i];
        if (c != ':') {
            kbstr_catf(&compiland->name, "%c", c);
        }
    }

    FILE * file = fopen(compiland->path.data, "rb");
    if (file == NULL) {
        perror("fopen");
        kbelog("could not open source file '%s'", compiland->path.data);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long int filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    kbstr_new(&compiland->content);

    if (filesize) {
        kbstr_resize(&compiland->content, filesize);
        size_t r = fread(compiland->content.data, filesize, 1, file);
        assert(r == 1);
    }
   
    fclose(file);
}

void kbcompiland_new_entry(struct kbcompiland* compiland, char* path) {
    kbcompiland_new_aux(compiland, path, true);
}

void kbcompiland_new(struct kbcompiland* compiland, char* path) {
    kbcompiland_new_aux(compiland, path, false);
}

void kbcompiland_new_virt(struct kbcompiland* compiland, char* path, char* content) {
    compiland->virtual = true;
    compiland->entry = false;
    compiland->boilerplate = false;

    kbstr_new(&compiland->path);
    kbstr_cat(&compiland->path, path);
    kbpath_normalize(&compiland->path);

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

    kbstr_new(&compiland->basename);
    kbstr_cat(&compiland->basename, path + base);
    kbstr_resize(&compiland->basename, compiland->basename.len - 3);

    kbstr_new(&compiland->name);
    for(int i = 0; i < compiland->path.len - 3; ++ i) {
        char c = (isds(compiland->path.data[i]))? '%' : compiland->path.data[i];
        kbstr_catf(&compiland->name, "%c", c);
    }

    kbstr_new(&compiland->content);
    kbstr_cat(&compiland->content, content);
}

void kbcompiland_del(struct kbcompiland * compiland) {
    kbstr_del(&compiland->path);
    kbstr_del(&compiland->basename);
    kbstr_del(&compiland->name);
    kbstr_del(&compiland->content);
}
