cat << END
#ifndef KBTOKEN__H
#define KBTOKEN__H

#define NUM_SPECIALS $(($(wc -l src/token.txt | awk '{printf $1}')+2))

extern char * specials[NUM_SPECIALS];

enum kbtoken_kind {
    NL,
END

while read line
do
    token=($line)
    cat << END
    ${token[0]},
END
done < src/token.txt

cat << END
    ILLEGAL,
};

struct kbtoken {
    enum kbtoken_kind kind;
    char * value;
    int line;
    int col;
};

struct kbtoken kbtoken_make(enum kbtoken_kind kind, char * value, int line, int col);

char * kbtoken_string(enum kbtoken_kind type);

void kbtoken_destroy_arr(unsigned int num_tokens, struct kbtoken * tokens);

void kbtoken_display(struct kbtoken * token);

void kbtoken_destroy(struct kbtoken * token);

int is_sep(char c);

int is_delim(char c);

#endif
END
