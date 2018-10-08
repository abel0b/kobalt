#ifndef KOBALT__LEXER__H
#define KOBALT__LEXER__H

#include "kobalt/lexer.h"
#include "array/array.h"
#include "kobalt/source_file.h"
#include <stdbool.h>

#define NKEYWORDS 8

struct KeywordMatcher {
    bool done;
    bool matched;
    int cursor;
    bool match[NKEYWORDS];
};

struct IntegerMatcher {
    bool first;
    bool is_integer;
};

struct FloatMatcher {
    bool is_float;
    bool has_dot;
    bool first;
};

enum LexerStateType {
    LEXER_NEWLINE,
    LEXER_INLINE
};

struct LexerState {
    enum LexerStateType type;
    int indent_level;
    bool newline;
    bool indent_tab;
    bool first_indent;
    int indent_counter;
    int space_indent;
    int line;
    struct KeywordMatcher keywords;
    struct IntegerMatcher integer_matcher;
    struct FloatMatcher float_matcher;
    int buffer_size;
    int cursor;
    char * buffer;
};

struct Array * lex (struct SourceFile * source_file);


struct KeywordMatcher keyword_matcher_make();

void keyword_matcher_next(struct KeywordMatcher * matcher, char ch);

struct FloatMatcher float_matcher_make();

void float_matcher_next(struct FloatMatcher * matcher, char ch);

struct IntegerMatcher integer_matcher_make();

void integer_matcher_next(struct IntegerMatcher * matcher, char ch);

struct LexerState * lexer_state_make();

void lexer_state_destroy(struct LexerState * state);

void lexer_state_next(struct LexerState * state, struct Array * tokens, char ch);

#endif
