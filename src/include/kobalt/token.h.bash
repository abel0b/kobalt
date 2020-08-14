cat << END
#ifndef KBTOKEN__H
#define KBTOKEN__H

#include "kobalt/kobalt.h"

#define NUM_SPECIALS $(($(wc -l src/token.csv | awk '{printf $1}')+2))

extern char * specials[NUM_SPECIALS];

enum kbtoken_kind {
    TNL,
END
while read line
do
    token=($line)
    cat << END
    T${token[0]},
END
done < src/token.csv

cat << END
    TILLEGAL,
};

struct kbtoken {
    enum kbtoken_kind kind;
    char * value;
    int line;
    int col;
};

struct kbtoken kbtoken_make(enum kbtoken_kind kind, char * value, int line, int col);

char * kbtoken_string(enum kbtoken_kind type);

void kbtoken_del_arr(unsigned int num_tokens, struct kbtoken * tokens);

void kbtoken_display(struct kbtoken * token);

void kbtoken_del(struct kbtoken * token);

int is_sep(char c);

int is_delim(char c);

#endif
END
