#ifndef KBCOMPILAND__H
#define KBCOMPILAND__H

#include "kobalt/kobalt.h"
#include "kobalt/options.h"
#include "kobalt/path.h"
#include <stdbool.h>

struct kbcompiland {
    bool virtual;
    struct kbstr path;
    struct kbstr name;
    struct kbstr basename;
    struct kbstr content;
    bool entry;
};

void kbcompiland_new_entry(struct kbcompiland* compiland, char* filename);

void kbcompiland_new_virt(struct kbcompiland* compiland, char* path, char* content);

void kbcompiland_del(struct kbcompiland* compiland);

#endif
