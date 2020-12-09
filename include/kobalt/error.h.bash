cat << END
#ifndef KLERROR__H
#define KLERROR__H

#include "kobalt/kobalt.h"

enum kl_err_level {
    WARNING,
    ERROR
};

enum kl_err_kind {
END

while read line
do
    token=($line)
    cat << END
    ${token[0]},
END
done < lib/kobalt/errors.csv
cat << END
};

struct kl_err {
    enum kl_err_kind kind;
    char * msg;
};

struct kl_errvec {
    int numerrs;
    int capacity;
    struct kl_err * errs;
};

char * kl_err_kind_str(enum kl_err_kind kind);
struct kl_err kl_err_make(enum kl_err_kind kind, char * msg);
void kl_err_display(struct kl_err * err);
void kl_err_del(struct kl_err * err);

struct kl_errvec kl_errvec_make();
void kl_errvec_push(struct kl_errvec * errvec, struct kl_err err);
void kl_errvec_shrink(struct kl_errvec * errvec, int diffsize);
void kl_errvec_display(struct kl_errvec * errvec);
void kl_errvec_del(struct kl_errvec * errvec);

#endif
END
