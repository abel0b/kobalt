#include "kobalt/lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

const char * keywords[NKEYWORDS] = {
    "int32",
    "float32",
    "if",
    "then",
    "else",
    "for",
    "while",
    "func",
    "void",
    "bool",
};

struct kblexer kblexer_make() {
    struct kblexer lexer;
    lexer.state = LEXER_NEWLINE;
    lexer.indent_level = 0;
    lexer.newline = true;
    lexer.indent_tab = true;
    lexer.first_indent = true;
    lexer.indent_counter = 0;
    lexer.space_indent = -1;
    lexer.line = 1;
    kblexer_kw_init(&lexer);
    kblexer_int_init(&lexer);
    kblexer_float_init(&lexer);
    lexer.buffer_size = 2;
    lexer.buffer = malloc(lexer.buffer_size * sizeof (char));
    lexer.cursor = 0;
    return lexer;
}

void kblexer_destroy(struct kblexer * lexer) {
    free(lexer->buffer);
}

void kblexer_kw_init(struct kblexer * lexer) {
    lexer->kw_match.done = false;
    lexer->kw_match.matched = true;
    lexer->kw_match.cursor = 0;
    for (int i = 0; i < NKEYWORDS; i++) {
        lexer->kw_match.match[i] = true;
    }
}

void kblexer_kw_next(struct kblexer * lexer, char ch) {
    if(lexer->kw_match.matched) {
        bool matched = false;
        for (int i = 0; i < NKEYWORDS; i++) {
            if (lexer->kw_match.match[i]) {
                if (strlen(keywords[i]) <= lexer->kw_match.cursor || keywords[i][lexer->kw_match.cursor] != ch) {
                    lexer->kw_match.match[i] = false;
                }
                else {
                    matched = true;
                }
            }
        }
        lexer->kw_match.matched = matched;
        lexer->kw_match.cursor ++;
    }
}

void kblexer_float_init(struct kblexer * lexer) {
    lexer->float_match.is_float = true;
    lexer->float_match.has_dot = false;
    lexer->float_match.first = true;
}

void kblexer_float_next(struct kblexer * lexer, char ch) {
    if (lexer->float_match.first) {
        lexer->float_match.first = false;
        if (ch == '-') return;
    }

    if (!lexer->float_match.has_dot && ch == '.') {
        lexer->float_match.has_dot = true;
        return;
    }

    if(!(ch >= '0' && ch <= '9')) {
        lexer->float_match.is_float = false;
    }
}

void kblexer_int_init(struct kblexer * lexer) {
    lexer->int_match.is_integer = true;
    lexer->int_match.first = true;
}

void kblexer_int_next(struct kblexer * lexer, char ch) {
    if (lexer->int_match.first) {
        lexer->int_match.first = false;
        if (ch == '-') return;
    }
    if(!(ch >= '0' && ch <= '9')) {
        lexer->int_match.is_integer = false;
    }
}

void kblexer_push(struct kbtoken ** tokens, unsigned int * num_tokens, unsigned int * capacity, struct kbtoken token) {
    if (*num_tokens == *capacity) {
        (*capacity) = (*capacity) * 2;
        *tokens = realloc(*tokens, sizeof(struct kbtoken) * (*capacity));
        assert(*tokens != NULL);
    }
    (*tokens)[*num_tokens] = token;
    (*num_tokens) ++;
}


bool is_sep(char ch) {
    return ch == '=' || ch == ' ' || ch == '\n' || ch == '(' || ch == ')' || ch == ':'; 
}

void kblexer_next(struct kblexer * lexer, struct kbtoken ** tokens, unsigned int * num_tokens, unsigned int * capacity, char ch) {
    switch(lexer->state) {
        case LEXER_NEWLINE:
            if (ch == ' ' || ch == '\t') {
                lexer->indent_counter ++;
            }
            else if (ch == '\n') {

            }
            else {
                if (lexer->first_indent && lexer->indent_counter) {
                    lexer->space_indent = lexer->indent_counter;
                    lexer->first_indent = false;
                }
                if (lexer->indent_counter == (lexer->indent_level + 1) * lexer->space_indent) {
                    lexer->indent_level ++;
                    kblexer_push(tokens, num_tokens, capacity, kbtoken_make(TOKEN_INDENT, NULL));
                }
                else if (lexer->indent_counter == (lexer->indent_level - 1) * lexer->space_indent) {
                    lexer->indent_level --;
                    kblexer_push(tokens, num_tokens, capacity, kbtoken_make(TOKEN_DEDENT, NULL));
                }
                lexer->indent_counter = 0;
                lexer->state = LEXER_NEWTOK;
                kblexer_next(lexer, tokens, num_tokens, capacity, ch);
            }
            break;
        case LEXER_NEWTOK:
            if (ch == '"') {
                lexer->state = LEXER_STRINGLIT;
            }
            else if (is_sep(ch)) {
                switch(ch) {
                    case '=':
                        kblexer_push(tokens, num_tokens, capacity, kbtoken_make(TOKEN_EQUAL, NULL));
                        break;
                    case '(':
                        kblexer_push(tokens, num_tokens, capacity, kbtoken_make(TOKEN_LPAR, NULL));
                        break;
                    case ')':
                        kblexer_push(tokens, num_tokens, capacity, kbtoken_make(TOKEN_RPAR, NULL));
                        break;
                    case ':':
                        kblexer_push(tokens, num_tokens, capacity, kbtoken_make(TOKEN_COLON, NULL));
                        break;
                    case '\n':
                        kblexer_push(tokens, num_tokens, capacity, kbtoken_make(TOKEN_LF, NULL));
                        lexer->state = LEXER_NEWLINE;
                        break;
                }
            }
            else if (ch >= '0' && ch <= '9') {
                lexer->state = LEXER_NUMLIT;
                kblexer_next(lexer, tokens, num_tokens, capacity, ch);
            }
            else if (ch == '/') {
                lexer->state = LEXER_COMMENT;
            }
            else if (ch == '\n') {
                lexer->state = LEXER_NEWLINE;
            }
            else if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z')) {
                lexer->state = LEXER_IDENTIFIER;
                kblexer_next(lexer, tokens, num_tokens, capacity, ch);
            }
            else {
                fprintf(stderr, "Unexpected token %c\n", ch);
                exit(1);
            }
            break;
        case LEXER_NUMLIT:
            if (ch >= '0' || ch <= '9' || ch == '.') {
                if (lexer->cursor == lexer->buffer_size) {
                    lexer->buffer_size = 2 * lexer->buffer_size;
                    lexer->buffer = realloc(lexer->buffer, lexer->buffer_size * sizeof(char));
                }
                lexer->buffer[lexer->cursor] = ch;
                lexer->cursor ++;
                kblexer_int_next(lexer, ch);
                kblexer_float_next(lexer, ch);
                if (!lexer->int_match.is_integer && !lexer->float_match.is_float) {
                    fprintf(stderr, "Unexpected token %c", ch);
                    exit(1);
                }
            }
            else if (is_sep(ch)) {
                if (lexer->int_match.is_integer) {
                    if (lexer->cursor == lexer->buffer_size) {
                        lexer->buffer_size = 2 * lexer->buffer_size;
                        lexer->buffer = realloc(lexer->buffer, lexer->buffer_size * sizeof(char));
                    }
                    lexer->buffer[lexer->cursor] = '\0';
                    kblexer_push(tokens, num_tokens, capacity, kbtoken_make(TOKEN_INTEGER, lexer->buffer));
                }
                else if (lexer->float_match.is_float) {
                    if (lexer->cursor == lexer->buffer_size) {
                        lexer->buffer_size = 2 * lexer->buffer_size;
                        lexer->buffer = realloc(lexer->buffer, lexer->buffer_size * sizeof(char));
                    }
                    lexer->buffer[lexer->cursor] = '\0';
                    kblexer_push(tokens, num_tokens, capacity, kbtoken_make(TOKEN_FLOAT, lexer->buffer));
                }
                kblexer_int_init(lexer);
                kblexer_float_init(lexer);
                lexer->cursor = 0;
                lexer->state = LEXER_NEWTOK;
                kblexer_next(lexer, tokens, num_tokens, capacity, ch);
            }
            break;
        case LEXER_IDENTIFIER:
            if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9')) {
                if (lexer->cursor == lexer->buffer_size) {
                    lexer->buffer_size = 2 * lexer->buffer_size;
                    lexer->buffer = realloc(lexer->buffer, lexer->buffer_size * sizeof(char));
                }
                lexer->buffer[lexer->cursor] = ch;
                lexer->cursor ++;
                kblexer_kw_next(lexer, ch);
            }
            else if (is_sep(ch)) {
                if (lexer->cursor == lexer->buffer_size) {
                    lexer->buffer_size = 2 * lexer->buffer_size;
                    lexer->buffer = realloc(lexer->buffer, lexer->buffer_size * sizeof(char));
                }
                lexer->buffer[lexer->cursor] = '\0';
                kblexer_push(tokens, num_tokens, capacity, kbtoken_make((lexer->kw_match.matched)?TOKEN_KEYWORD:TOKEN_IDENTIFIER, lexer->buffer));
                kblexer_kw_init(lexer);
                lexer->cursor = 0;
                lexer->state = LEXER_NEWTOK;
                kblexer_next(lexer, tokens, num_tokens, capacity, ch);
            }
            else {
                fprintf(stderr, "Unexpected token %c\n", ch);
                exit(1);
            }
            break;
        case LEXER_STRINGLIT:
            if (ch == '"') {
                lexer->buffer[lexer->cursor] = '\0';
                kblexer_push(tokens, num_tokens, capacity, kbtoken_make(TOKEN_STRLIT, lexer->buffer));
                lexer->cursor = 0;
                lexer->state = LEXER_NEWTOK;
            }
            else {
                if (lexer->cursor == lexer->buffer_size) {
                    lexer->buffer_size = 2 * lexer->buffer_size;
                    lexer->buffer = realloc(lexer->buffer, lexer->buffer_size * sizeof(char));
                }
                lexer->buffer[lexer->cursor] = ch;
                lexer->cursor++;
            }
            break;
        case LEXER_COMMENT:
            if (ch == '\n') {
                lexer->state = LEXER_NEWLINE;
            }
            break;
    }
}

struct kbtoken * kblexer_start(struct kblexer * lexer, struct kbsrc * src, unsigned int * num_tokens) { 
    *num_tokens = 0;
    unsigned int capacity = 2;
    struct kbtoken * tokens = malloc(sizeof(struct kbtoken) * capacity);
    for (int i = 0; i < src->length; i++) {
        kblexer_next(lexer, &tokens, num_tokens, &capacity, src->content[i]);
    }

    while (lexer->indent_level-- > 0) {
        tokens[*num_tokens] = kbtoken_make(TOKEN_DEDENT, NULL);
        (*num_tokens)++;
    }

    return tokens;
}
