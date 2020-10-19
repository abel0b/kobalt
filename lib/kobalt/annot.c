#include "kobalt/annot.h"
#include "kobalt/memory.h"
#include "kobalt/log.h"

void kbannotctx_new(struct kbannotctx* ctx) {
    ctx->num_annotkinds = 0;
    ctx->cap_annotkinds = 2;
    ctx->annotkinds = (struct kbannotkind*)kbmalloc(sizeof(*ctx->annotkinds) * ctx->cap_annotkinds);
}

void kbannotctx_add(struct kbannotctx* ctx, int id, size_t size, void (*annot_new)(void*), void (*annot_del)(void*)) {
    if (ctx->num_annotkinds == ctx->cap_annotkinds) {
        ctx->cap_annotkinds *= 2;
        ctx->annotkinds = (struct kbannotkind*)kbrealloc(ctx->annotkinds, sizeof(*ctx->annotkinds) * ctx->cap_annotkinds);
    }
    struct kbannotkind* annotkind = &ctx->annotkinds[ctx->num_annotkinds ++];
    annotkind->id = id;
    annotkind->size = size;
    annotkind->annot_new = annot_new;
    annotkind->annot_del = annot_del;
}

void kbannotctx_del(struct kbannotctx* ctx) {
    kbfree(ctx->annotkinds);
}

void kbannot_new(struct kbannot* annot) {
    annot->num_annots = 0;
    annot->cap_annots = 0;
    annot->annots = NULL;
    annot->ids = NULL;
}

int kbannot_has(struct kbannotctx* ctx, struct kbannot* annot, int id) {
    struct kbannotkind* annotkind = NULL;
    for(int i = 0; i < ctx->num_annotkinds; ++i) {
        if(ctx->annotkinds[i].id == id) {
            annotkind = &ctx->annotkinds[i];
            break;
        }
    }
    if (annotkind == NULL) {
        kbelog("undefined annotation '%d'", id);
        exit(1);
    }
    int i;
    for(i = 0; i < annot->num_annots; ++i) {
        if (annot->ids[i] == id) break;
    }
    return i < annot->num_annots;
}

void* kbannot_get(struct kbannotctx* ctx, struct kbannot* annot, int id) {
    struct kbannotkind* annotkind = NULL;
    for(int i = 0; i < ctx->num_annotkinds; ++i) {
        if(ctx->annotkinds[i].id == id) {
            annotkind = &ctx->annotkinds[i];
            break;
        }
    }
    if (annotkind == NULL) {
        kbelog("undefined annotation '%d'", id);
        exit(1);
    }
    int i;
    for(i = 0; i < annot->num_annots; ++i) {
        if (annot->ids[i] == id) break;
    }
    if (i == annot->num_annots) {
        if (annot->cap_annots == annot->num_annots) {
            annot->cap_annots = (annot->cap_annots == 0)? 2: 2 * annot->cap_annots;
            annot->ids = (int*)kbrealloc(annot->ids, sizeof(annot->ids[0]) * annot->cap_annots);
            annot->annots = (void**) kbrealloc(annot->annots, sizeof(annot->annots[0]) * annot->cap_annots);
        }
        annot->ids[annot->num_annots] = id;
        annot->annots[annot->num_annots] = kbmalloc(annotkind->size);
        annotkind->annot_new(annot->annots[annot->num_annots]);
        ++ annot->num_annots;
    }
    return annot->annots[i];
}

void kbannot_del(struct kbannotctx* ctx, struct kbannot* annot) {
    for(int i = 0; i < annot->num_annots; ++i) {
        struct kbannotkind* annotkind = NULL;
        for(int j = 0; j < ctx->num_annotkinds; ++j) {
            if(ctx->annotkinds[j].id == annot->ids[i]) {
                annotkind = &ctx->annotkinds[j];
                break;
            }
        }
        annotkind->annot_del(annot->annots[i]);
        kbfree(annot->annots[i]);
    }
    kbfree(annot->ids);
    kbfree(annot->annots);
}
