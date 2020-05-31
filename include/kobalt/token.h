#ifndef KBTOKEN__H
#define KBTOKEN__H

#define NUM_SPECIALS 54

extern char * specials[NUM_SPECIALS];

enum kbtoken_kind {
    NL,
    IDENTIFIER,
    FLOAT,
    INT,
    STR,
    CHAR,
    INDENT,
    DEDENT,
    END,
    COMMENT,
    SEMI,
    COLON,
    LPAR,
    RPAR,
    LBRACK,
    RBRACK,
    PLUS,
    MINUS,
    STAR,
    DASH,
    SLASH,
    PERCENT,
    CARET,
    EXCLAM,
    QUERY,
    AND,
    OR,
    ANDAND,
    LSHIFT,
    RSHIFT,
    EQ,
    EQEQ,
    PLUSEQ,
    MINUSEQ,
    STAREQ,
    SLASHEQ,
    PERCENTEQ,
    CARETEQ,
    ANDEQ,
    OREQ,
    LSHIFTEQ,
    RSHIFTEQ,
    NEQ,
    GT,
    LT,
    GEQ,
    LEQ,
    AT,
    DOT,
    DOTDOT,
    DOTDOTDOT,
    DOTDOTEQ,
    RARROW,
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
