#ifndef KBLEXER__H
#define KBLEXER__H

#include <stdbool.h>
#include "kobalt/source.h"
#include "kobalt/token.h"

extern char * keywords[];

enum kblexer_state {
    LEXER_NEWLINE,
    LEXER_NEWTOK,
    LEXER_SYM,
    LEXER_STRINGLIT,
    LEXER_NUMLIT,
    LEXER_COMMENT,
    LEXER_IDENTIFIER,
};

struct kblexer {
    enum kblexer_state state;
    int indent_level;
    bool newline;
    bool indent_tab;
    bool first_indent;
    bool use_tabs_indent;
    bool first_indent_char;
    int indent_counter;
    int space_indent;
    int line;
    int col;
    int tokline;
    int tokcol;
    struct  {
        int cursor;
        bool match[NUM_SPECIALS];
        int num_matched;
        int matched;
    } special_match;
    struct {
        bool first;
        bool is_integer;
    } int_match;
    struct {
        bool is_float;
        bool has_dot;
        bool first;
    } float_match;
    int buffer_size;
    int cursor;
    char * buffer;
};

struct kblexer kblexer_make();

void kblexer_destroy(struct kblexer * state);

void kblexer_special_init(struct kblexer * lexer);

void kblexer_special_next(struct kblexer * lexer, char ch);

void kblexer_float_init(struct kblexer * lexer);

void kblexer_float_next(struct kblexer * lexer, char ch);

void kblexer_int_init(struct kblexer * lexer);

void kblexer_int_next(struct kblexer * lexer, char ch);

void kblexer_next(struct kblexer * lexer, struct kbtoken ** tokens, unsigned int * num_tokens, unsigned int * capacity, char ch);

struct kbtoken * kblexer_start(struct kblexer * lexer, struct kbsrc * src, unsigned int * num_tokens);

#endif
