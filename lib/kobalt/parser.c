#include "kobalt/parser.h"
#include "abl/mem.h"
#include "kobalt/ast.h"
#include "abl/log.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct kl_parser {
    struct abl_vec_token* tokens;
    int curnode;
    struct kl_ast* ast;
    struct kl_errvec errvec;
    struct kl_compiland* compiland;
    int capacity;
    int numnodes;
    int cursor;
};

struct kl_parser kl_parser_make(struct kl_token* tokens, struct kl_compiland* compiland);

void kl_parser_new(struct kl_parser* parser, struct abl_vec_token* tokens, struct kl_compiland* compiland, struct kl_ast* ast);

void kl_parser_run(struct kl_parser* parser);

void kl_parser_del(struct kl_parser * parser);

/* Get attribute of a given node */
#define attr(P,N,A) (P->ast->nodes.data[N].data.N.A)

/* Check whether the parser is in an error state */
static int reject(struct kl_parser* parser) {
    return parser->errvec.numerrs;
}

/* Returns if the parser is in an error state */
int bubble(struct kl_parser* parser, int nid) {
    if (reject(parser)) {
#ifdef DEBUG
        assert(nid == -1);
#endif
        return nid;
    }
    if (nid != -1) {
        parser->curnode = parser->ast->nodes.data[nid].parent;
    }
    return nid;
}

/* Check whether a token should be ignored */
static int ignore(enum kl_token_kind tokind) {
    return tokind == TComment;
}

#define errcheck(P) do { if(reject(P)) return bubble(P, -1); } while(0)

#define defrule(R) static int make_##R(struct kl_parser* parser)

defrule(todo);
defrule(id);
defrule(type);
defrule(lit);
defrule(call);
defrule(expr);
defrule(callparam);
defrule(callparams);
defrule(val);
defrule(funparam);
defrule(funparams);
defrule(ifelse);
defrule(forloop);


/* Look ahead token stream without consuming */
static struct kl_token * lookahead(struct kl_parser* parser, int stride, enum kl_token_kind expected_kind) {
    enum kl_token_kind actual_kind;
    while(parser->cursor + stride < parser->tokens->size && ignore(actual_kind = parser->tokens->data[parser->cursor + stride].kind)) {
        parser->cursor++;
    }
    struct kl_token* tok = (parser->cursor + stride < parser->tokens->size)? &parser->tokens->data[parser->cursor + stride] : NULL;
#ifdef DEBUG
    if(getenv("DEBUG_PARSER") && tok) {
        if (tok->value) {
            abl_dlog(RED "lookahead" RESET " %d %s? %s:\"%s\"", stride, kl_token_string(expected_kind), kl_token_string(actual_kind), tok->value);
        }
        else {
            abl_dlog(RED "lookahead" RESET " %d %s? %s", stride, kl_token_string(expected_kind), kl_token_string(actual_kind));
        }
    }
#endif
    return (actual_kind == expected_kind)? tok : NULL;
}

static struct kl_token * anylookahead(struct kl_parser* parser, int stride) {
    enum kl_token_kind actual_kind;
    while(parser->cursor + stride < parser->tokens->size && ignore(actual_kind = parser->tokens->data[parser->cursor+stride].kind)) {
        ++ parser->cursor;
    }
    if (parser->cursor + stride >= parser->tokens->size) {
        abl_elog("parser: bad cursor");
        exit(1);
    }
    struct kl_token* tok = (parser->cursor + stride < parser->tokens->size)? &parser->tokens->data[parser->cursor + stride] : NULL;

#ifdef DEBUG
    if(getenv("DEBUG_PARSER") && tok) {
        if (tok->value) {
            abl_dlog(RED "lookahead" RESET " %d %s:\"%s\"", stride, kl_token_string(actual_kind), tok->value);
        }
        else {
            abl_dlog(RED "lookahead" RESET " %d %s", stride, kl_token_string(actual_kind));
        }
    }
#endif
    return tok;
}

static struct kl_token * lookaheadid(struct kl_parser* parser, int stride, char* id) {
    struct kl_token * idtok = lookahead(parser, stride, TId);
    if(idtok && !strcmp(idtok->value, id)) return idtok;
    return NULL;
}

static struct kl_token * peek(struct kl_parser* parser, enum kl_token_kind expected_kind) {
    return lookahead(parser, 0, expected_kind);
}

static struct kl_token * anypeek(struct kl_parser* parser) {
    return anylookahead(parser, 0);
}

static struct kl_token * peekid(struct kl_parser* parser, char* id) {
    return lookaheadid(parser, 0, id);
}

static struct kl_token * anyeat(struct kl_parser* parser) {
    if (parser->cursor >= parser->tokens->size) {
        abl_elog("unexpected state");
        exit(1);
    }
    struct kl_token* tok = &parser->tokens->data[parser->cursor++];
#ifdef DEBUG
        if(getenv("DEBUG_PARSER")) {
            abl_dlog(RED "consume" RESET " %s", kl_token_string(tok->kind));
        }
#endif
    return tok;
}

#if DEBUG
static struct kl_token * unchecked_eat(struct kl_parser* parser, enum kl_token_kind kind, char* file, int line) {
#else
static struct kl_token * unchecked_eat(struct kl_parser* parser, enum kl_token_kind kind) {
#endif
    struct kl_token * tok = NULL;
    if (peek(parser, kind)) {
        tok = &parser->tokens->data[parser->cursor];
#ifdef DEBUG
        if(getenv("DEBUG_PARSER")) {
            abl_dlog(RED "consume" RESET " %s", kl_token_string(kind));
        }
#endif
    }
    else {
        int bufsize = 128;
        char * strbuf = abl_malloc(128 * bufsize);
        int size;
#if DEBUG
    while ((size = snprintf(strbuf, bufsize, "(%s:%d) Unexpected token at %s:%d:%d\nExpected %s but got %s\n", file, line, parser->compiland->path.data, parser->tokens->data[parser->cursor].loc.line, parser->tokens->data[parser->cursor].loc.col, kl_token_string(kind), kl_token_string(parser->tokens->data[parser->cursor].kind))) > bufsize) {
#else
    while ((size = snprintf(strbuf, bufsize, "Unexpected token at %s:%d:%d\nExpected %s but got %s\n", parser->compiland->path.data, parser->tokens->data[parser->cursor].loc.line, parser->tokens->data[parser->cursor].loc.col, kl_token_string(kind), kl_token_string(parser->tokens->data[parser->cursor].kind))) > bufsize) {
#endif
            bufsize = size+1;
            strbuf = abl_realloc(strbuf, (size+1) * sizeof(char));
        }
        kl_errvec_push(&parser->errvec, kl_err_make(ESYNTAX, strbuf));
#ifdef DEBUG
    if(getenv("DEBUG_PARSER")) {
        abl_dlog(RED "consume" RESET " expected:%s actual:%s", kl_token_string(kind), kl_token_string(parser->tokens->data[parser->cursor].kind));
    }
#endif
    }
    parser->cursor++;
    return tok;
}

#if DEBUG
#define eat(P,K) unchecked_eat(P,K,__FILE__,__LINE__);errcheck(P)
#else
#define eat(P,K) unchecked_eat(P,K);errcheck(P)
#endif

static struct kl_token * unchecked_eatid(struct kl_parser* parser, char * id) {
    struct kl_token * tok = NULL;
    if (peek(parser, TId)) {
        if (!strcmp(parser->tokens->data[parser->cursor].value, id)) {
            tok = &parser->tokens->data[parser->cursor];
#if DEBUG
            if(getenv("DEBUG_PARSER")) {
                abl_dlog(RED "consumeid" RESET " ID:%s", id);
            }
#endif
        }
#if DEBUG
        else {
            if(getenv("DEBUG_PARSER")) {
                abl_dlog(RED "consumeid" RESET " expected:%s actual:%s", id, parser->tokens->data[parser->cursor].value);
            }
        }
#endif
    }
    if(!tok) {
        int bufsize = 128;
        char * strbuf = abl_malloc(128 * bufsize);
        int size;
        while ((size = snprintf(strbuf, bufsize, "Unexpected identifier at %s:%d:%d\nExpected %s but got %s\n", parser->compiland->path.data, parser->tokens->data[parser->cursor].loc.line, parser->tokens->data[parser->cursor].loc.col, id, parser->tokens->data[parser->cursor].value)) > bufsize) {
            bufsize = size+1;
            strbuf = abl_realloc(strbuf, (size+1) * sizeof(char));
        }
        kl_errvec_push(&parser->errvec, kl_err_make(ESYNTAX, strbuf));
#ifdef DEBUG
        if(getenv("DEBUG_PARSER")) {
            abl_dlog(RED "consumeid" RESET " expected:%s=%s actual:%s", kl_token_string(TId), id, kl_token_string(parser->tokens->data[parser->cursor].kind));
        }
#endif
    }

    parser->cursor++;
    return tok;
}

#define eatid(P,K) unchecked_eatid(P, K);errcheck(P)

static int unchecked_make(struct kl_parser* parser, int nid) {
    (void)parser;
#if DEBUG
    if (nid != -1 && getenv("DEBUG_PARSER")) {
        abl_dlog(RED "<<< make" RESET " %s", kl_node_kind_str(parser->ast->nodes.data[nid].kind));
    }
#endif
    return nid;
}

#define make(P,R) unchecked_make(P,R);errcheck(P)

static void unchecked_push_make(struct kl_parser* parser, int group, int recres) {
#if DEBUG
    if(getenv("DEBUG_PARSER")) {
        abl_dlog(RED "<<< push_make" RESET " %s", kl_node_kind_str(parser->ast->nodes.data[group].kind));
    }
    assert(isgroup(&parser->ast->nodes.data[group]) );
#endif
    if (recres == -1) return;
    if (attr(parser, group, numitems) == attr(parser, group, itemscap)) {
        attr(parser, group, itemscap) = (attr(parser, group, itemscap) > 0)? attr(parser, group, itemscap) * 2 : 1;
        attr(parser, group, items) = abl_realloc(attr(parser, group, items), sizeof(attr(parser, group, items)[0]) * attr(parser, group, itemscap));
    }
    attr(parser, group, items)[attr(parser, group, numitems)] = recres;
    ++ attr(parser, group, numitems);
}

#define push_make(P,C,R) unchecked_push_make(P,C,R);errcheck(P)

/* Allocate a new ast node */
static int kl_parser_addnode(struct kl_parser* parser, enum kl_node_kind kind) {
#if DEBUG
    if(getenv("DEBUG_PARSER")) {
        abl_dlog(RED ">>> newnode" RESET " %s", kl_node_kind_str(kind));
    }
#endif
    struct kl_token* token = anypeek(parser);
    int newnode = kl_ast_add(parser->ast, kind, parser->curnode, token->loc);
    parser->curnode = parser->numnodes;
    return newnode;
}

int is_sym(struct kl_token* tok) {
    return (tok->kind == TSemi)
        || (tok->kind == TColon)
        || (tok->kind == TColonColon)
        || (tok->kind == TLPar)
        || (tok->kind == TRPar)
        || (tok->kind == TLBrack)
        || (tok->kind == TRBrack)
        || (tok->kind == TLCurly)
        || (tok->kind == TRCurly)
        || (tok->kind == TPlus)
        || (tok->kind == TStar)
        || (tok->kind == TDash)
        || (tok->kind == TSlash)
        || (tok->kind == TPercent)
        || (tok->kind == TCaret)
        || (tok->kind == TExclam)
        || (tok->kind == TQuery)
        || (tok->kind == TAnd)
        || (tok->kind == TOr)
        || (tok->kind == TAndAnd)
        || (tok->kind == TLShift)
        || (tok->kind == TRShift)
        || (tok->kind == TEq)
        || (tok->kind == TEqEq)
        || (tok->kind == TPlusEq)
        || (tok->kind == TDashEq)
        || (tok->kind == TStarEq)
        || (tok->kind == TSlashEq)
        || (tok->kind == TPercentEq)
        || (tok->kind == TCaretEq)
        || (tok->kind == TAndEq)
        || (tok->kind == TOrEq)
        || (tok->kind == TLShiftEq)
        || (tok->kind == TRShiftEq)
        || (tok->kind == TExclamEq)
        || (tok->kind == TGT)
        || (tok->kind == TLT)
        || (tok->kind == TGEq)
        || (tok->kind == TLEq)
        || (tok->kind == TAt)
        || (tok->kind == TDot)
        || (tok->kind == TDotDot)
        || (tok->kind == TDotDotDot)
        || (tok->kind == TDotDotEq);
}

static int make_todo(struct kl_parser* parser) {
    printf("TODO\n");
    kl_errvec_push(&parser->errvec, kl_err_make(ETODO, "Unimplemented grammar rule"));
    return bubble(parser, -1);
}

static int make_id(struct kl_parser* parser) {
    int id = kl_parser_addnode(parser, NId);
    struct kl_token * tokid = eat(parser, TId);
#ifdef DEBUG
    assert(tokid->value);
#endif
    attr(parser, id, name) = abl_malloc((strlen(tokid->value) + 1) * sizeof(char));
    strcpy(attr(parser, id, name), tokid->value);
    return bubble(parser, id);
}

static int make_id_str(struct kl_parser* parser, char* str) {
    int id = kl_parser_addnode(parser, NId);
    attr(parser, id, name) = abl_malloc((strlen(str) + 1) * sizeof(char));
    strcpy(attr(parser, id, name), str);
    return bubble(parser, id);
}

static int make_sym(struct kl_parser* parser) {
    int id = kl_parser_addnode(parser, NId);
    struct kl_token* tok = anypeek(parser);
    if (is_sym(tok)) {
        anyeat(parser);
        attr(parser, id, name) = abl_malloc((strlen(specials[tok->kind]) + 1) * sizeof(attr(parser, id, name)[0]));
        strcpy(attr(parser, id, name), specials[tok->kind]);
    }
    else {
        abl_elog("expected sym");
        exit(1);
        return bubble(parser, -1);
    }
    return bubble(parser, id);
}

static int make_type_str(struct kl_parser* parser, char* str) {
    int  type = kl_parser_addnode(parser, NType);
    attr(parser, type, name) = abl_malloc((strlen(str) + 1) * sizeof(char));
    strcpy(attr(parser, type, name), str);
    return bubble(parser, type);
}

static int make_funparam(struct kl_parser* parser) {
    int funparam = kl_parser_addnode(parser, NFunParam);

    if (peek(parser, TUnit)) {
        eat(parser, TUnit);
        attr(parser, funparam, id) = make(parser, make_id_str(parser, "_"));
        attr(parser, funparam, type) = make(parser, make_type_str(parser, "()"));
    }
    else if (peek(parser, TId)) {
        attr(parser, funparam, id) = make(parser, make_id(parser));
        attr(parser, funparam, type) = -1;
    }
    else {
        eat(parser, TLPar);
        attr(parser, funparam, id) = make(parser, make_id(parser));
        eat(parser, TColonColon);
        attr(parser, funparam, type) = make(parser, make_type(parser));
        eat(parser, TRPar);
    }

    return bubble(parser, funparam);
}

static int make_funparam_unit(struct kl_parser* parser) {
    int funparam = kl_parser_addnode(parser, NFunParam);
    attr(parser, funparam, id) = make(parser, make_id_str(parser, "_"));
    attr(parser, funparam, type) = make(parser, make_type_str(parser, "()"));
    return bubble(parser, funparam);
}

static int make_type(struct kl_parser* parser) {
    int  type = kl_parser_addnode(parser, NType);
    
    if (peek(parser, TUnit)) {
        eat(parser, TUnit);
        attr(parser, type, name) = abl_malloc((strlen("()") + 1) * sizeof(char));
        strcpy(attr(parser, type, name), "()");
    }
    else {
        struct kl_token * tokid = eat(parser, TId);
        attr(parser, type, name) = abl_malloc((strlen(tokid->value) + 1) * sizeof(char));
        strcpy(attr(parser, type, name), tokid->value);
    }

    return bubble(parser, type);
}

static int make_strlit(struct kl_parser* parser) {
    int strlit = kl_parser_addnode(parser, NStrLit);
    struct kl_token * tok = eat(parser, TStr);
    attr(parser, strlit, value) = abl_malloc(strlen(tok->value) + 1);
    strcpy(attr(parser, strlit, value), tok->value);
    return bubble(parser, strlit);
}

static int make_intlit(struct kl_parser* parser) {
    int intlit = kl_parser_addnode(parser, NIntLit);
    struct kl_token * tok = eat(parser, TInt);
    attr(parser, intlit, value) = abl_malloc(strlen(tok->value) + 1);
    strcpy(attr(parser, intlit, value), tok->value);
    return bubble(parser, intlit);
}

static int make_floatlit(struct kl_parser* parser) {
    int floatlit = kl_parser_addnode(parser, NFloatLit);
    struct kl_token * tok = eat(parser, TFloat);
    attr(parser, floatlit, value) = abl_malloc(strlen(tok->value) + 1);
    strcpy(attr(parser, floatlit, value), tok->value);
    return bubble(parser, floatlit);
}

static int make_charlit(struct kl_parser* parser) {
    int charlit = kl_parser_addnode(parser, NCharLit);
    struct kl_token * tok = eat(parser, TChar);
    attr(parser, charlit, value) = abl_malloc(strlen(tok->value) + 1);
    strcpy(attr(parser, charlit, value), tok->value);
    return bubble(parser, charlit);
}

static int make_lit(struct kl_parser* parser) {
    if(peek(parser, TStr)) {
        return make_strlit(parser);
    }
    else if (peek(parser, TInt)) {
        return make_intlit(parser);
    }
    else if (peek(parser, TFloat)) {
        return make_floatlit(parser);
    }
    else if (peek(parser, TChar)) {
        return make_charlit(parser);
    }
    return bubble(parser, -1);
}

static int make_import(struct kl_parser* parser) {
    int import = kl_parser_addnode(parser, NImport);
    eat(parser, TId);
    struct kl_token * tok = eat(parser, TId);
    attr(parser, import, path) = abl_malloc(strlen(tok->value) + 1);
    strcpy(attr(parser, import, path), tok->value);
    return bubble(parser, import);
}

static int make_term(struct kl_parser* parser) {
    struct kl_token* tok = anypeek(parser);
    if(tok->kind == TStr || tok->kind == TInt || tok->kind == TFloat || tok->kind == TChar) {
        return make_lit(parser);
    }
    else if (peek(parser, TLPar)) {
        eat(parser, TLPar);
        int expr = make_expr(parser);
        eat(parser, TRPar);
        return bubble(parser, expr);
    }
    else if (peek(parser, TId)) {
        return make_id(parser);
    }
    else {
        return bubble(parser, -1);
    }
}

static int make_call(struct kl_parser* parser) {
    int call = kl_parser_addnode(parser, NCall);
    if (is_builtin_fun(anypeek(parser)->kind)) {
        attr(parser, call, id) = make(parser, make_sym(parser));
    }
    else {
        attr(parser, call, id) = make(parser, make_id(parser));
    }
    attr(parser, call, callparams) = make(parser, make_callparams(parser));
    return bubble(parser, call);
}

static int make_seq(struct kl_parser* parser) {
    int seq = kl_parser_addnode(parser, NSeq);
    eat(parser, TLineFeed);
    eat(parser, TIndent);

    if (peek(parser, TDedent)) {
        // TODO: handle error
        abl_elog("empty sequence");
        exit(1);
    }
    
    do {
        push_make(parser, seq, make_expr(parser));
        if(peek(parser, TLineFeed)) eat(parser, TLineFeed);
    } while(!peek(parser, TDedent));

    eat(parser, TDedent);
    return bubble(parser, seq);
}

static int make_expr(struct kl_parser* parser) {
    int expr;
    if (peek(parser, TLineFeed)) {
        expr = make_seq(parser);
    }
    else if (peek(parser, TLPar) || lookahead(parser, 1, TLineFeed) || lookahead(parser, 1, TDedent) || lookaheadid(parser, 1, "then") || lookaheadid(parser, 1, "else") || lookaheadid(parser, 1, "elif") || lookahead(parser, 1, TRPar)) {
        expr = make_term(parser);
    }
    else if (peekid(parser, "val")) {
        expr = make(parser, make_val(parser));
    }
    else if (peekid(parser, "if")) {
        expr = make(parser, make_ifelse(parser));
    }
    else if (peekid(parser, "for")) {
        expr = make(parser, make_forloop(parser));
    }
    else if (peekid(parser, "case")) {
        expr = make(parser, make_todo(parser));
    }
    else {
        expr = make(parser, make_call(parser));
    }
    
    return bubble(parser, expr);
}

static int make_callparam(struct kl_parser* parser) {
    int callparam = kl_parser_addnode(parser, NCallParam);
    attr(parser, callparam, expr) = make(parser, make_term(parser));
    return bubble(parser, callparam);
}

static int make_callparams(struct kl_parser* parser) {
    int callparams = kl_parser_addnode(parser, NCallParams);
    while(!peek(parser, TLineFeed) && !peekid(parser, "then") && !peekid(parser, "else") && !peek(parser, TRPar) && !peekid(parser, "elif")) {
        push_make(parser, callparams, make_callparam(parser));
    }
    return bubble(parser, callparams);
}

static int make_val(struct kl_parser* parser) {
    int val = kl_parser_addnode(parser, NVal);
    eatid(parser, "val");

    attr(parser, val, id) = make(parser, make_id(parser));
    eat(parser, TEq);
    attr(parser, val, expr) = make(parser, make_expr(parser));

    return bubble(parser, val);
}

static int make_forloop(struct kl_parser* parser) {
    int forloop = kl_parser_addnode(parser, NForLoop);
    eatid(parser, "for");
    attr(parser, forloop, id) = make(parser, make_id(parser));
    eatid(parser, "in");
    attr(parser, forloop, start) = make(parser, make_intlit(parser));
    eat(parser, TDotDot);
    attr(parser, forloop, end) = make(parser, make_intlit(parser));
    eatid(parser, "do");
    attr(parser, forloop, expr) = make(parser, make_expr(parser));

    return bubble(parser, forloop);
}

static int make_funparams(struct kl_parser* parser) {
    int funparams = kl_parser_addnode(parser, NFunParams);

    while(!peek(parser, TColonColon) && !peek(parser, TEq) && !peek(parser, TLineFeed)) {
        push_make(parser, funparams, make_funparam(parser));
    }
    return bubble(parser, funparams);
}

static int make_funparams_unit(struct kl_parser* parser) {
    int funparams = kl_parser_addnode(parser, NFunParams);

    push_make(parser, funparams, make_funparam_unit(parser));
    return bubble(parser, funparams);
}

static int make_fun(struct kl_parser* parser) {
    int fun = kl_parser_addnode(parser, NFun);
    eatid(parser, "fun");
    attr(parser, fun, id) = make(parser, make_id(parser));
    
    if (peek(parser, TEq) || peek(parser, TColonColon) || peek(parser, TLineFeed)) {
        attr(parser, fun, funparams) = make(parser, make_funparams_unit(parser));
    }
    else {
        attr(parser, fun, funparams) = make(parser, make_funparams(parser));
    }

    if (peek(parser, TColonColon)) {
        eat(parser, TColonColon);
        attr(parser, fun, rettype) = make(parser, make_type(parser));
    }
    else {
        attr(parser, fun, rettype) = -1;
    }

    if (peek(parser, TEq)) {
        eat(parser, TEq);
        attr(parser, fun, body) = make(parser, make_expr(parser));
    }
    else {
        attr(parser, fun, body) = -1;
    }
    return bubble(parser, fun);
}

static int make_ifelse(struct kl_parser* parser) {
    int ifelse = kl_parser_addnode(parser, NIfElse);
    {
        int ifbranch = kl_parser_addnode(parser, NIfBranch);
        eatid(parser, "if");
        attr(parser, ifbranch, cond) = make(parser, make_expr(parser));
        push_make(parser, ifelse, ifbranch);
        eatid(parser, "then");
        attr(parser, ifbranch, conseq) = make(parser, make_expr(parser));
    }
    
    while(peekid(parser, "elif")) {
        eatid(parser, "elif");
        int ifbranch = kl_parser_addnode(parser, NElifBranch);
        attr(parser, ifbranch, cond) = make(parser, make_expr(parser));
        push_make(parser, ifelse, ifbranch);
        eatid(parser, "then");
        attr(parser, ifbranch, conseq) = make(parser, make_expr(parser));
    }
    
    if (peekid(parser, "else")) {
        eatid(parser, "else");
        int ifbranch = kl_parser_addnode(parser, NElseBranch);
        attr(parser, ifbranch, cond) = -1;
        push_make(parser, ifelse, ifbranch);
        attr(parser, ifbranch, conseq) = make(parser, make_expr(parser));
    }
    return bubble(parser, ifelse);
}

static int make_boilerplate(struct kl_parser* parser, int group) {
    int import = kl_parser_addnode(parser, NImport);
    attr(parser, import, path) = abl_malloc(strlen("std") + 1);
    strcpy(attr(parser, import, path), "std");
    push_make(parser, group, import);
    return 1;
}

static int make_program(struct kl_parser* parser) {
    int group = kl_parser_addnode(parser, NProgram);
    
    if (parser->compiland->boilerplate) {
        make_boilerplate(parser, group);
    }

    while (!peek(parser, TEndFile)) {
        if (peekid(parser, "fun")) {
            push_make(parser, group, make_fun(parser));
        }
        else if (peekid(parser, "import")) {
            push_make(parser, group, make_import(parser));
        }
        else {
            push_make(parser, group, make_expr(parser));
        }
        if (peek(parser, TLineFeed)) {
            eat(parser, TLineFeed);
        }
    }
    eat(parser, TEndFile);
    return bubble(parser, group);
}

void kl_parser_new(struct kl_parser* parser, struct abl_vec_token* tokens, struct kl_compiland* compiland, struct kl_ast* ast) {
    parser->curnode = -1;
    parser->capacity = 0;
    parser->numnodes = 0;
    parser->cursor = 0;
    parser->tokens = tokens;
    parser->compiland = compiland;
    parser->ast = ast;
    parser->errvec = kl_errvec_make();
    kl_ast_new(parser->ast);
}

void kl_parser_del(struct kl_parser* parser) {
    kl_errvec_del(&parser->errvec);
}

void kl_parse(struct abl_vec_token* tokens, struct kl_compiland* compiland, struct kl_ast* ast) {
    struct kl_parser parser;
    kl_parser_new(&parser, tokens, compiland, ast);
    kl_parser_run(&parser);
    kl_parser_del(&parser);
}

void kl_parser_run(struct kl_parser* parser) {
    int group = make_program(parser);
#ifdef DEBUG
    if(getenv("DEBUG_PARSER")) {
        abl_dlog(RED "DONE " RESET "%d nodes", parser->numnodes);
    }
#endif
    if (group) {
        kl_errvec_display(&parser->errvec);
        exit(1);
    }
}
