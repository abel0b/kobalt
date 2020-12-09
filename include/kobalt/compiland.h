#ifndef KLCOMPILAND__H
#define KLCOMPILAND__H

#include "kobalt/kobalt.h"
#include "kobalt/options.h"
#include "klbase/path.h"
#include <stdbool.h>

struct kl_compiland {
    bool virtual;
    bool boilerplate;
    struct kl_str path;
    struct kl_str name;
    struct kl_str basename;
    struct kl_str content;
    bool entry;
};

void kl_compiland_new_entry(struct kl_compiland* compiland, char* path);

void kl_compiland_new(struct kl_compiland* compiland, char* path);

void kl_compiland_new_virt(struct kl_compiland* compiland, char* path, char* content);

void kl_compiland_del(struct kl_compiland* compiland);

#endif
