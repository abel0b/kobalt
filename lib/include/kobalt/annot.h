#ifndef KBANNOT_H
#define KBANNOT_H

#include "kobalt/kobalt.h"
#include <stdlib.h>

struct kbannotkind {
    int id;
    size_t size;
    void (*annot_new)(void*);
    void (*annot_del)(void*);
};  

struct kbannotctx {
    int num_annotkinds;
    int cap_annotkinds;
    struct kbannotkind* annotkinds;
};

void kbannotctx_new(struct kbannotctx* ctx);
void kbannotctx_add(struct kbannotctx* ctx, int id, size_t size, void (*annot_new)(void*), void (*annot_del)(void*));
void kbannotctx_del(struct kbannotctx* ctx);

struct kbannot {
    int* ids;
    void** annots;
    int num_annots;
    int cap_annots;
};

void kbannot_new(struct kbannot* annots);
int kbannot_has(struct kbannotctx* ctx, struct kbannot* annot, int id);
void* kbannot_get(struct kbannotctx* ctx, struct kbannot* annots, int id);
void kbannot_del(struct kbannotctx* ctx, struct kbannot* annots);

#endif
