#ifndef KBTOKEN__H
#define KBTOKEN__H

#include "kobalt/kobalt.h"
#include "kobalt/vec.h"
#include <stdio.h>

#define NUM_SPECIALS 59

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
    TColonColon,
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

struct kbloc {
    int line;
    int col;
};

struct kbtoken {
    enum kbtoken_kind kind;
    char* value;
    struct kbloc loc;
};

kbvec_decl(struct kbtoken, token)

int is_builtin_fun(enum kbtoken_kind tok_kind);

struct kbtoken kbtoken_make(enum kbtoken_kind kind, char* value, int line, int col);

char * kbtoken_string(enum kbtoken_kind type);

void kbtoken_del_arr(unsigned int num_tokens, struct kbtoken* tokens);

void kbtoken_display(FILE* file, struct kbtoken* token);

void kbtoken_del(struct kbtoken* token);

int is_sep(char c);

int is_delim(char c);

#endif
