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
    "",
    "",
    ";",
    "#",
    ":",
    "()",
    "(",
    ")",
    "[",
    "]",
    "{",
    "}",
    "+",
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
    ":=",
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

void kbtoken_del_arr(unsigned int num_tokens, struct kbtoken * tokens) {
    for(unsigned int ii=0; ii<num_tokens; ii++) {
        if(tokens[ii].value != NULL) free(tokens[ii].value);
    }
    free(tokens);
}

void kbtoken_display(FILE* file, struct kbtoken* token) {
    fprintf(file, "%s", kbtoken_string(token->kind));
    if (token->value != NULL) {
        fprintf(file, "=\"%s\"", token->value);
    }
    fprintf(file, " at %d:%d\n", token->line, token->col);
}

void kbtoken_del(struct kbtoken * token) {
    if (token->value != NULL)
        free (token->value);
}

char * kbtoken_string(enum kbtoken_kind kind) {
    switch(kind) {
        case TNL:
            return "NL";
        case TId:
            return "Id";
        case TFloat:
            return "Float";
        case TInt:
            return "Int";
        case TStr:
            return "Str";
        case TChar:
            return "Char";
        case TIndent:
            return "Indent";
        case TDedent:
            return "Dedent";
        case TEndFile:
            return "EndFile";
        case TLineFeed:
            return "LineFeed";
        case TComment:
            return "Comment";
        case TSemi:
            return "Semi";
        case THash:
            return "Hash";
        case TColon:
            return "Colon";
        case TUnit:
            return "Unit";
        case TLPar:
            return "LPar";
        case TRPar:
            return "RPar";
        case TLBrack:
            return "LBrack";
        case TRBrack:
            return "RBrack";
        case TLCurly:
            return "LCurly";
        case TRCurly:
            return "RCurly";
        case TPlus:
            return "Plus";
        case TStar:
            return "Star";
        case TDash:
            return "Dash";
        case TSlash:
            return "Slash";
        case TPercent:
            return "Percent";
        case TCaret:
            return "Caret";
        case TExclam:
            return "Exclam";
        case TQuery:
            return "Query";
        case TAnd:
            return "And";
        case TOr:
            return "Or";
        case TAndAnd:
            return "AndAnd";
        case TLShift:
            return "LShift";
        case TRShift:
            return "RShift";
        case TAssign:
            return "Assign";
        case TEq:
            return "Eq";
        case TEqEq:
            return "EqEq";
        case TPlusEq:
            return "PlusEq";
        case TDashEq:
            return "DashEq";
        case TStarEq:
            return "StarEq";
        case TSlashEq:
            return "SlashEq";
        case TPercentEq:
            return "PercentEq";
        case TCaretEq:
            return "CaretEq";
        case TAndEq:
            return "AndEq";
        case TOrEq:
            return "OrEq";
        case TLShiftEq:
            return "LShiftEq";
        case TRShiftEq:
            return "RShiftEq";
        case TExclamEq:
            return "ExclamEq";
        case TGT:
            return "GT";
        case TLT:
            return "LT";
        case TGEq:
            return "GEq";
        case TLEq:
            return "LEq";
        case TAt:
            return "At";
        case TDot:
            return "Dot";
        case TDotDot:
            return "DotDot";
        case TDotDotDot:
            return "DotDotDot";
        case TDotDotEq:
            return "DotDotEq";
        case TILLEGAL:
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
        || c == '#'
        || c == '!'
        || c == '^'
        || c == ']'
        || c == '['
        || c == '@'
        || c == '}'
        || c == '|'
        || c == '{'
    ;
}
