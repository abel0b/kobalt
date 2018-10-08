#ifndef KOBALT__TOKEN__H
#define KOBALT__TOKEN__H

#include "array/array.h"

enum TokenType {
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_STRING_LITERAL,
    TOKEN_INDENT,
    TOKEN_DEDENT,
    TOKEN_LF,
    TOKEN_LPAR,
    TOKEN_RPAR,
    TOKEN_COLON,
    TOKEN_EQUAL,
    TOKEN_KEYWORD
};

struct Token {
    enum TokenType type;
    char * value;
    int size;
};

struct Token * token_make(enum TokenType type, char * value);

void token_array_destroy(struct Array * tokens);

char * token_string(enum TokenType type);

void token_debug(struct Token * token);

void token_destroy(struct Token * token);

#endif
