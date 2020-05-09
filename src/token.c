#include "kobalt/token.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct kbtoken kbtoken_make(enum kbtoken_kind kind, char * value) {
    struct kbtoken token;
    token.kind = kind;
    if (value == NULL) {
        token.value = NULL;
    }
    else {
        token.value = malloc(strlen(value)+1);
        strcpy(token.value, value);
    }
    return token;
}

char * kbtoken_string(enum kbtoken_kind kind) {
    switch(kind) {
        case TOKEN_IDENTIFIER:
            return "IDENTIFIER";
            break;
        case TOKEN_INTEGER:
            return "INTEGER";
            break;
        case TOKEN_FLOAT:
            return "FLOAT";
            break;
        case TOKEN_STRLIT:
            return "STRLIT";
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

void kbtoken_debug(struct kbtoken * token) {
    printf("%s", kbtoken_string(token->kind));
    if (token->value != NULL) {
        printf(" %s", token->value);
    }
    printf("\n");
}

void kbtoken_destroy(struct kbtoken * token) {
    if (token->value != NULL)
        free (token->value);
}
