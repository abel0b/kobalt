#include "kobalt/error.h"
#include "kobalt/memory.h"
#include <stdio.h>

char * kberr_kind_str(enum kberr_kind kind) {
    switch(kind) {
        case ESYNTAX:
            return "ESYNTAX";
    }
    return "UNDEFINED";
}

struct kberrvec kberrvec_make() {
    struct kberrvec errvec = {
        .numerrs = 0,
        .capacity = 0,
        .errs = NULL,
    };
    return errvec;
}

struct kberr kberr_make(enum kberr_kind kind, char * msg) {
    struct kberr err = {
        .kind = kind,
        .msg = msg,
    };
    return err;
}

void kberr_destroy(struct kberr * err) {
    kbfree(err->msg);
}

void kberrvec_push(struct kberrvec * errvec, struct kberr err) {
    if (errvec->numerrs == errvec->capacity) {
        errvec->capacity = errvec->capacity*2 + 2*(errvec->capacity == 0);
        errvec->errs = kbrealloc(errvec->errs, 0, sizeof(struct kberr) * errvec->capacity);
    }
    errvec->errs[errvec->numerrs++] = err;
}

void kberrvec_destroy(struct kberrvec * errvec) {
    for(int ii=0; ii<errvec->numerrs; ++ii) kberr_destroy(&errvec->errs[ii]);
    if (errvec->errs) kbfree(errvec->errs);
}

void kberrvec_shrink(struct kberrvec * errvec, int diffsize) {
    errvec->numerrs -= diffsize;
}

void kberr_display(struct kberr * err) {
    fprintf(stderr, "%s %s\n", kberr_kind_str(err->kind), err->msg);
}

void kberrvec_display(struct kberrvec * errvec) {
    for(int ii=0; ii<errvec->numerrs; ++ii) {
        kberr_display(&errvec->errs[ii]);
    }
}
