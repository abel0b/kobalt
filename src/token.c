#include "kobalt/token.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char * specials[NUM_SPECIALS] = {
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "--",
    ";",
    ":",
    "(",
    ")",
    "[",
    "]",
    "+",
    "-",
    "*",
    "-",
    "/",
    "%",
    "^",
    "!",
    "?",
    "&",
    "|",
    "&&",
    "<<",
    ">>",
    "=",
    "==",
    "+=",
    "-=",
    "*=",
    "/=",
    "%=",
    "^=",
    "&=",
    "|=",
    "<<=",
    ">>=",
    "!=",
    ">",
    "<",
    ">=",
    "<=",
    "@",
    ".",
    "..",
    "...",
    "..=",
    "->",
    "",
};

struct kbtoken kbtoken_make(enum kbtoken_kind kind, char * value, int line, int col) {
    struct kbtoken token;
    token.kind = kind;
    if (value == NULL) {
        token.value = NULL;
    }
    else {
        token.value = malloc(strlen(value)+1);
        strcpy(token.value, value);
    }
    token.line = line;
    token.col = col;
    return token;
}

void kbtoken_destroy_arr(unsigned int num_tokens, struct kbtoken * tokens) {
    for(unsigned int ii=0; ii<num_tokens; ii++) {
        if(tokens[ii].value != NULL) free(tokens[ii].value);
    }
    free(tokens);
}

void kbtoken_display(struct kbtoken * token) {
    printf("%s", kbtoken_string(token->kind));
    if (token->value != NULL) {
        printf("=\"%s\"", token->value);
    }
    printf(" at %d:%d\n", token->line, token->col);
}

void kbtoken_destroy(struct kbtoken * token) {
    if (token->value != NULL)
        free (token->value);
}

char * kbtoken_string(enum kbtoken_kind kind) {
    switch(kind) {
        case NL:
            return "NL";
        case IDENTIFIER:
            return "IDENTIFIER";
        case FLOAT:
            return "FLOAT";
        case INT:
            return "INT";
        case STR:
            return "STR";
        case CHAR:
            return "CHAR";
        case INDENT:
            return "INDENT";
        case DEDENT:
            return "DEDENT";
        case END:
            return "END";
        case COMMENT:
            return "COMMENT";
        case SEMI:
            return "SEMI";
        case COLON:
            return "COLON";
        case LPAR:
            return "LPAR";
        case RPAR:
            return "RPAR";
        case LBRACK:
            return "LBRACK";
        case RBRACK:
            return "RBRACK";
        case PLUS:
            return "PLUS";
        case MINUS:
            return "MINUS";
        case STAR:
            return "STAR";
        case DASH:
            return "DASH";
        case SLASH:
            return "SLASH";
        case PERCENT:
            return "PERCENT";
        case CARET:
            return "CARET";
        case EXCLAM:
            return "EXCLAM";
        case QUERY:
            return "QUERY";
        case AND:
            return "AND";
        case OR:
            return "OR";
        case ANDAND:
            return "ANDAND";
        case LSHIFT:
            return "LSHIFT";
        case RSHIFT:
            return "RSHIFT";
        case EQ:
            return "EQ";
        case EQEQ:
            return "EQEQ";
        case PLUSEQ:
            return "PLUSEQ";
        case MINUSEQ:
            return "MINUSEQ";
        case STAREQ:
            return "STAREQ";
        case SLASHEQ:
            return "SLASHEQ";
        case PERCENTEQ:
            return "PERCENTEQ";
        case CARETEQ:
            return "CARETEQ";
        case ANDEQ:
            return "ANDEQ";
        case OREQ:
            return "OREQ";
        case LSHIFTEQ:
            return "LSHIFTEQ";
        case RSHIFTEQ:
            return "RSHIFTEQ";
        case NEQ:
            return "NEQ";
        case GT:
            return "GT";
        case LT:
            return "LT";
        case GEQ:
            return "GEQ";
        case LEQ:
            return "LEQ";
        case AT:
            return "AT";
        case DOT:
            return "DOT";
        case DOTDOT:
            return "DOTDOT";
        case DOTDOTDOT:
            return "DOTDOTDOT";
        case DOTDOTEQ:
            return "DOTDOTEQ";
        case RARROW:
            return "RARROW";
        case ILLEGAL:
            return "ILLEGAL";
    }
    return "UNDEFINED";
}

int is_delim(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n';
}

int is_sep(char c) {
    return c == '?'
        || c == '>'
        || c == '='
        || c == '<'
        || c == ';'
        || c == ':'
        || c == '/'
        || c == '.'
        || c == '-'
        || c == '+'
        || c == '*'
        || c == ')'
        || c == '('
        || c == '&'
        || c == '%'
        || c == '!'
        || c == '^'
        || c == ']'
        || c == '['
        || c == '@'
        || c == '|'
    ;
}
