#ifndef KLCOMPILAND__H
#define KLCOMPILAND__H

#include "kobalt/kobalt.h"
#include "kobalt/options.h"
#include "abl/path.h"
#include <stdbool.h>

struct kl_compiland {
    bool virtual;
    bool boilerplate;
    struct abl_str path;
    struct abl_str name;
    struct abl_str basename;
    struct abl_str content;
    bool entry;
};

void kl_compiland_new_entry(struct kl_compiland* compiland, char* path);

void kl_compiland_new(struct kl_compiland* compiland, char* path);

void kl_compiland_new_virt(struct kl_compiland* compiland, char* path, char* content);

void kl_compiland_del(struct kl_compiland* compiland);

#endif
