cat << END
#ifndef KLTOKEN__H
#define KLTOKEN__H

#include "kobalt/kobalt.h"
#include "klbase/vec.h"
#include <stdio.h>

#define NUM_SPECIALS $(($(wc -l lib/kobalt/token.csv | awk '{printf $1}')+2))

extern char * specials[NUM_SPECIALS];

// TODO: bitmasks for tokens

enum kl_token_kind {
    TNL,
END
while read line
do
    token=($line)
    cat << END
    T${token[0]},
END
done < lib/kobalt/token.csv

cat << END
    TILLEGAL,
};

struct kl_loc {
    int line;
    int col;
};

struct kl_token {
    enum kl_token_kind kind;
    char* value;
    struct kl_loc loc;
};

kl_vec_decl(struct kl_token, token)

int is_builtin_fun(enum kl_token_kind tok_kind);

struct kl_token kl_token_make(enum kl_token_kind kind, char* value, int line, int col);

char * kl_token_string(enum kl_token_kind type);

void kl_token_del_arr(unsigned int num_tokens, struct kl_token* tokens);

void kl_token_display(FILE* file, struct kl_token* token);

void kl_token_del(struct kl_token* token);

int is_sep(char c);

int is_delim(char c);

#endif
END
