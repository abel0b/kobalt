#ifndef KBLEXER__H
#define KBLEXER__H

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
    int newline;
    int indent_tab;
    int first_indent;
    int use_tabs_indent;
    int first_indent_char;
    int indent_counter;
    int space_indent;
    int line;
    int col;
    int tokline;
    int tokcol;
    struct  {
        int cursor;
        int match[NUM_SPECIALS];
        int nummatched;
        int matched;
    } special_match;
    struct {
        int first;
        int is_integer;
    } int_match;
    struct {
        int is_float;
        int has_dot;
        int first;
    } float_match;
    int buffer_size;
    int cursor;
    char * buffer;
};

void kblexer_new(struct kblexer* lexer);

void kblexer_del(struct kblexer* lexer);

void kblexer_special_init(struct kblexer* lexer);

void kblexer_special_next(struct kblexer* lexer, char ch);

void kblexer_float_init(struct kblexer* lexer);

void kblexer_float_next(struct kblexer* lexer, char ch);

void kblexer_int_init(struct kblexer* lexer);

void kblexer_int_next(struct kblexer* lexer, char ch);

void kblexer_next(struct kblexer* lexer, struct kbtoken** tokens, int* numtokens, int* capacity, char ch);

void kblexer_run(struct kblexer* lexer, struct kbsrc* src, struct kbtoken** tokens, int* numtokens); 

void kblex(struct kbsrc* src, struct kbtoken** tokens, int* numtokens);

#endif
