#include "kobalt/token.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct Token * token_make(enum TokenType type, char * value) {
    struct Token * token = malloc (sizeof (struct Token));
    token->type = type;
    if (value == NULL) {
        token->value = NULL;
    }
    else {
        token->value = strdup(value);
    }

    return token;
}

char * token_string(enum TokenType type) {
    switch(type) {
        case TOKEN_IDENTIFIER:
            return "IDENTIFIER";
            break;
        case TOKEN_INTEGER:
            return "INTEGER";
            break;
        case TOKEN_FLOAT:
            return "FLOAT";
            break;
        case TOKEN_STRING_LITERAL:
            return "STRING_LITERAL";
            break;
        case TOKEN_INDENT:
            return "INDENT";
            break;
        case TOKEN_DEDENT:
            return "DEDENT";
            break;
        case TOKEN_LF:
            return "LF";
            break;
        case TOKEN_LPAR:
            return "LPAR";
            break;
        case TOKEN_RPAR:
            return "RPAR";
            break;
        case TOKEN_COLON:
            return "COLON";
            break;
        case TOKEN_EQUAL:
            return "EQUAL";
            break;
        case TOKEN_KEYWORD:
            return "KEYWORD";
            break;
        default:
            break;
    }
    return "UNDEFINED";
}

void token_array_destroy(struct Array * tokens) {
    struct ArrayIterator it = array_it_make(tokens);
    struct Token * token;
    while(!array_it_end(it)) {
        token = array_it_get(it);
        free(token->value);
        free(token);
        it = array_it_next(it);
    }
    array_destroy(tokens);
}

void token_debug(struct Token * token) {
    printf("%s", token_string(token->type));
    if (token->value != NULL) {
        printf(" %s", token->value);
    }
    printf("\n");
}

void token_destroy(struct Token * token) {
    if (token->value)
        free (token->value);
    free (token);
}
