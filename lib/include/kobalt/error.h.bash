cat << END
#ifndef KBERROR__H
#define KBERROR__H

#include "kobalt/kobalt.h"

enum kberr_level {
    WARNING,
    ERROR
};

enum kberr_kind {
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

struct kberr {
    enum kberr_kind kind;
    char * msg;
};

struct kberrvec {
    int numerrs;
    int capacity;
    struct kberr * errs;
};

char * kberr_kind_str(enum kberr_kind kind);
struct kberr kberr_make(enum kberr_kind kind, char * msg);
void kberr_display(struct kberr * err);
void kberr_del(struct kberr * err);

struct kberrvec kberrvec_make();
void kberrvec_push(struct kberrvec * errvec, struct kberr err);
void kberrvec_shrink(struct kberrvec * errvec, int diffsize);
void kberrvec_display(struct kberrvec * errvec);
void kberrvec_del(struct kberrvec * errvec);

#endif
END
