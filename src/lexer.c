#include "kobalt/lexer.h"
#include "kobalt/token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char * keywords[NKEYWORDS] = {
    "int",
    "float",
    "if",
    "then",
    "else",
    "do",
    "done",
    "while"
};

struct KeywordMatcher keyword_matcher_make() {
    struct KeywordMatcher matcher;
    matcher.done = false;
    matcher.matched = true;
    matcher.cursor = 0;
    for (int i = 0; i < NKEYWORDS; i++) {
        matcher.match[i] = true;
    }
    return matcher;
}

void keyword_matcher_next(struct KeywordMatcher * matcher, char ch) {
    if(matcher->matched) {
        bool matched = false;
        for (int i = 0; i < NKEYWORDS; i++) {
            if (matcher->match[i]) {
                if (strlen(keywords[i]) <= matcher->cursor || keywords[i][matcher->cursor] != ch) {
                    matcher->match[i] = false;
                }
                else {
                    matched = true;
                }
            }
        }
        matcher->matched = matched;
        matcher->cursor ++;
    }
}

struct LexerState * lexer_state_make() {
    struct LexerState * state = malloc (sizeof (struct LexerState));
    state->type = LEXER_NEWLINE;
    state->indent_level = 0;
    state->newline = true;
    state->indent_tab = true;
    state->first_indent = true;
    state->indent_counter = 0;
    state->space_indent = -1;
    state->line = 1;
    state->keywords = keyword_matcher_make();
    state->integer_matcher = integer_matcher_make();
    state->float_matcher = float_matcher_make();
    state->buffer_size = 2;
    state->buffer = malloc (state->buffer_size * sizeof (char));
    state->cursor = 0;

    return state;
}

void lexer_state_destroy(struct LexerState * state) {
    free(state->buffer);
    free (state);
}

struct FloatMatcher float_matcher_make() {
    struct FloatMatcher matcher;
    matcher.is_float = true;
    matcher.has_dot = false;
    matcher.first = true;
    return matcher;
}

struct IntegerMatcher integer_matcher_make() {
    struct IntegerMatcher matcher;
    matcher.is_integer = true;
    matcher.first = true;
    return matcher;
}

void float_matcher_next(struct FloatMatcher * matcher, char ch) {
    if (matcher->first) {
        matcher->first = false;
        if (ch == '-') return;
    }

    if (!matcher->has_dot && ch == '.') {
        matcher->has_dot = true;
        return;
    };

    if(!(ch >= '0' && ch <= '9')) {
        matcher->is_float = false;
    }
}

void integer_matcher_next(struct IntegerMatcher * matcher, char ch) {
    if (matcher->first) {
        matcher->first = false;
        if (ch == '-') return;
    }
    if(!(ch >= '0' && ch <= '9')) {
        matcher->is_integer = false;
    }
}

void lexer_state_next(struct LexerState * state, struct Array * tokens, char ch) {
    switch(state->type) {
        case LEXER_NEWLINE:
            if (ch == ' ' || ch == '\t') {
                state->indent_counter ++;
            }
            else {
                if (state->first_indent && state->indent_counter) {
                    state->space_indent = state->indent_counter;
                    state->first_indent = false;
                }
                if (state->indent_counter == (state->indent_level + 1) * state->space_indent) {
                    state->indent_level ++;
                    array_push(tokens, token_make(TOKEN_INDENT, NULL));
                }
                else if (state->indent_counter == (state->indent_level - 1) * state->space_indent) {
                    state->indent_level --;
                    array_push(tokens, token_make(TOKEN_DEDENT, NULL));
                }
                state->indent_counter = 0;
                state->type = LEXER_INLINE;
                lexer_state_next(state, tokens, ch);
            }
            break;
        case LEXER_INLINE:
            if (ch == '=' || ch == ' ' || ch == '\n' || ch == '(' || ch == ')') {
                state->buffer[state->cursor] = '\0';
                if (state->cursor) {
                    if(state->keywords.matched) {
                        array_push(tokens, token_make(TOKEN_KEYWORD, state->buffer));
                    }
                    else if(state->integer_matcher.is_integer) {
                        array_push(tokens, token_make(TOKEN_INTEGER, state->buffer));
                    }
                    else if(state->float_matcher.is_float) {
                        array_push(tokens, token_make(TOKEN_FLOAT, state->buffer));
                    }
                    else {
                        array_push(tokens, token_make(TOKEN_IDENTIFIER, state->buffer));
                    }
                }
                state->cursor = 0;
                switch(ch) {
                    case '=':
                        array_push(tokens, token_make(TOKEN_EQUAL, NULL));
                        break;
                    case '(':
                        array_push(tokens, token_make(TOKEN_LPAR, NULL));
                        break;
                    case ')':
                        array_push(tokens, token_make(TOKEN_RPAR, NULL));
                        break;
                    case '\n':
                        array_push(tokens, token_make(TOKEN_LF, NULL));
                        state->type = LEXER_NEWLINE;
                        break;
                }
                state->keywords = keyword_matcher_make();
                state->integer_matcher = integer_matcher_make();
                state->float_matcher = float_matcher_make();
                break;
            }
            else {
                state->buffer[state->cursor] = ch;
                keyword_matcher_next(&state->keywords, ch);
                integer_matcher_next(&state->integer_matcher, ch);
                float_matcher_next(&state->float_matcher, ch);
                state->cursor++;
                if (state->cursor == state->buffer_size) {
                    state->buffer_size = 2 * state->buffer_size;
                    state->buffer = realloc(state->buffer, state->buffer_size * sizeof(char));
                }
            }
            break;
    }
}

struct Array * lex (struct SourceFile * source_file) {
    printf("== lex %s ==\n", source_file->filename);

    struct Array * tokens = array_make();
    struct LexerState * state = lexer_state_make();

    for (int i = 0; i < source_file->length; i++) {
        lexer_state_next(state, tokens, source_file->content[i]);
    }
    while (state->indent_level-- > 0) {
        array_push(tokens, token_make(TOKEN_DEDENT, NULL));
    }

    lexer_state_destroy(state);

    return tokens;
}
