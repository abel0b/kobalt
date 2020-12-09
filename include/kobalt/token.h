#ifndef KLTOKEN__H
#define KLTOKEN__H

#include "kobalt/kobalt.h"
#include "klbase/vec.h"
#include <stdio.h>

#define NUM_SPECIALS 59

extern char * specials[NUM_SPECIALS];

// TODO: bitmasks for tokens

enum kl_token_kind {
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

struct kl_loc {
    int line;
    int col;
};

struct kl_token {
    enum kl_token_kind kind;
    char* value;
    struct kl_loc loc;
};

kl_vec_decl(struct kl_token, token)

int is_builtin_fun(enum kl_token_kind tok_kind);

struct kl_token kl_token_make(enum kl_token_kind kind, char* value, int line, int col);

char * kl_token_string(enum kl_token_kind type);

void kl_token_del_arr(unsigned int num_tokens, struct kl_token* tokens);

void kl_token_display(FILE* file, struct kl_token* token);

void kl_token_del(struct kl_token* token);

int is_sep(char c);

int is_delim(char c);

#endif
