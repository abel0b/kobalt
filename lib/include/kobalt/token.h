#ifndef KBTOKEN__H
#define KBTOKEN__H

#include "kobalt/kobalt.h"
#include <stdio.h>

#define NUM_SPECIALS 58

extern char * specials[NUM_SPECIALS];

// TODO: bitmasks for tokens

enum kbtoken_kind {
    TNL,
    TId,
    TFloat,
    TInt,
    TStr,
    TChar,
    TIndent,
    TDedent,
    TEndFile,
    TLineFeed,
    TComment,
    TSemi,
    THash,
    TColon,
    TUnit,
    TLPar,
    TRPar,
    TLBrack,
    TRBrack,
    TLCurly,
    TRCurly,
    TPlus,
    TStar,
    TDash,
    TSlash,
    TPercent,
    TCaret,
    TExclam,
    TQuery,
    TAnd,
    TOr,
    TAndAnd,
    TLShift,
    TRShift,
    TAssign,
    TEq,
    TEqEq,
    TPlusEq,
    TDashEq,
    TStarEq,
    TSlashEq,
    TPercentEq,
    TCaretEq,
    TAndEq,
    TOrEq,
    TLShiftEq,
    TRShiftEq,
    TExclamEq,
    TGT,
    TLT,
    TGEq,
    TLEq,
    TAt,
    TDot,
    TDotDot,
    TDotDotDot,
    TDotDotEq,
    TILLEGAL,
};

struct kbtoken {
    enum kbtoken_kind kind;
    char * value;
    int line;
    int col;
};

struct kbtoken kbtoken_make(enum kbtoken_kind kind, char* value, int line, int col);

char * kbtoken_string(enum kbtoken_kind type);

void kbtoken_del_arr(unsigned int num_tokens, struct kbtoken* tokens);

void kbtoken_display(FILE* file, struct kbtoken* token);

void kbtoken_del(struct kbtoken* token);

int is_sep(char c);

int is_delim(char c);

#endif
