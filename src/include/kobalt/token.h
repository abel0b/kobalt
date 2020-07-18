#ifndef KBTOKEN__H
#define KBTOKEN__H

#define NUM_SPECIALS 56

extern char * specials[NUM_SPECIALS];

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
    TComment,
    TDashDash,
    TSemi,
    TColon,
    TLPar,
    TRPar,
    TLBrack,
    TRBrack,
    TLCurly,
    TRCurly,
    TPlus,
    TMinus,
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
    TEq,
    TEqEq,
    TPlusEq,
    TMinusEq,
    TStarEq,
    TSlashEq,
    TPercentEq,
    TCaretEq,
    TAndEq,
    TOrEq,
    TLShiftEq,
    TRShiftEq,
    TNeq,
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

struct kbtoken kbtoken_make(enum kbtoken_kind kind, char * value, int line, int col);

char * kbtoken_string(enum kbtoken_kind type);

void kbtoken_del_arr(unsigned int num_tokens, struct kbtoken * tokens);

void kbtoken_display(struct kbtoken * token);

void kbtoken_del(struct kbtoken * token);

int is_sep(char c);

int is_delim(char c);

#endif
