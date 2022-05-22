cat << END
#include "kobalt/error.h"
#include "abl/mem.h"
#include <stdio.h>

char * kl_err_kind_str(enum kl_err_kind kind) {
    switch(kind) {
END
while read line
do
    tok=($line)
    cat << END
        case ${tok[0]}:
            return "${tok[0]}";
END
done < lib/kobalt/errors.csv

cat << END
    }
    return "UNDEFINED";
}

struct kl_errvec kl_errvec_make() {
    struct kl_errvec errvec;
    errvec.numerrs = 0;
    errvec.capacity = 0;
    errvec.errs = NULL;
    return errvec;
}

struct kl_err kl_err_make(enum kl_err_kind kind, char * msg) {
    struct kl_err err;
    err.kind = kind;
    err.msg = msg;
    return err;
}

void kl_err_del(struct kl_err * err) {
    abl_free(err->msg);
}

void kl_errvec_push(struct kl_errvec * errvec, struct kl_err err) {
    if (errvec->numerrs == errvec->capacity) {
        errvec->capacity = errvec->capacity*2 + 2*(errvec->capacity == 0);
        errvec->errs = abl_realloc(errvec->errs, sizeof(struct kl_err) * errvec->capacity);
    }
    errvec->errs[errvec->numerrs++] = err;
}

void kl_errvec_del(struct kl_errvec * errvec) {
    for(int ii=0; ii<errvec->numerrs; ++ii) kl_err_del(&errvec->errs[ii]);
    if (errvec->errs) abl_free(errvec->errs);
}

void kl_errvec_shrink(struct kl_errvec * errvec, int diffsize) {
    errvec->numerrs -= diffsize;
}

void kl_err_display(struct kl_err * err) {
    fprintf(stderr, "%s %s\n", kl_err_kind_str(err->kind), err->msg);
}

void kl_errvec_display(struct kl_errvec * errvec) {
    int ii;
    for(ii=0; ii<errvec->numerrs; ++ii) {
        kl_err_display(&errvec->errs[ii]);
    }
}
END
