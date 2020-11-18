#include "kobalt/parser.h"
#include "kobalt/memory.h"
#include "kobalt/ast.h"
#include "kobalt/log.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* Get attribute of a given node */
#define attr(P,N,A) (P->ast->nodes.data[N].data.N.A)

/* Check whether the parser is in an error state */
static int reject(struct kbparser* parser) {
    return parser->errvec.numerrs;
}

/* Returns if the parser is in an error state */
int bubble(struct kbparser* parser, int nid) {
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
static int ignore(enum kbtoken_kind tokind) {
    return tokind == TComment;
}

#define errcheck(P) do { if(reject(P)) return bubble(P, -1); } while(0)

#define defrule(R) static int make_##R(struct kbparser* parser)

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

/* Look ahead token stream without consuming */
static struct kbtoken * lookahead(struct kbparser* parser, int stride, enum kbtoken_kind expected_kind) {
    enum kbtoken_kind actual_kind;
    while(parser->cursor + stride < parser->tokens->size && ignore(actual_kind = parser->tokens->data[parser->cursor + stride].kind)) {
        parser->cursor++;
    }
    struct kbtoken* tok = (parser->cursor + stride < parser->tokens->size)? &parser->tokens->data[parser->cursor + stride] : NULL;
#ifdef DEBUG
    if(getenv("DEBUG_PARSER") && tok) {
        if (tok->value) {
            kbdlog(RED "lookahead" RESET " %d %s? %s:\"%s\"", stride, kbtoken_string(expected_kind), kbtoken_string(actual_kind), tok->value);
        }
        else {
            kbdlog(RED "lookahead" RESET " %d %s? %s", stride, kbtoken_string(expected_kind), kbtoken_string(actual_kind));
        }
    }
#endif
    return (actual_kind == expected_kind)? tok : NULL;
}

static struct kbtoken * anylookahead(struct kbparser* parser, int stride) {
    enum kbtoken_kind actual_kind;
    while(parser->cursor + stride < parser->tokens->size && ignore(actual_kind = parser->tokens->data[parser->cursor+stride].kind)) {
        ++ parser->cursor;
    }
    if (parser->cursor + stride >= parser->tokens->size) {
        kbelog("parser: bad cursor");
        exit(1);
    }
    struct kbtoken* tok = (parser->cursor + stride < parser->tokens->size)? &parser->tokens->data[parser->cursor + stride] : NULL;

#ifdef DEBUG
    if(getenv("DEBUG_PARSER") && tok) {
        if (tok->value) {
            kbdlog(RED "lookahead" RESET " %d %s:\"%s\"", stride, kbtoken_string(actual_kind), tok->value);
        }
        else {
            kbdlog(RED "lookahead" RESET " %d %s", stride, kbtoken_string(actual_kind));
        }
    }
#endif
    return tok;
}

static struct kbtoken * lookaheadid(struct kbparser* parser, int stride, char* id) {
    struct kbtoken * idtok = lookahead(parser, stride, TId);
    if(idtok && !strcmp(idtok->value, id)) return idtok;
    return NULL;
}

static struct kbtoken * peek(struct kbparser* parser, enum kbtoken_kind expected_kind) {
    return lookahead(parser, 0, expected_kind);
}

static struct kbtoken * anypeek(struct kbparser* parser) {
    return anylookahead(parser, 0);
}

static struct kbtoken * peekid(struct kbparser* parser, char* id) {
    return lookaheadid(parser, 0, id);
}

static struct kbtoken * anyeat(struct kbparser* parser) {
    if (parser->cursor >= parser->tokens->size) {
        kbelog("unexpected state");
        exit(1);
    }
    struct kbtoken* tok = &parser->tokens->data[parser->cursor++];
#ifdef DEBUG
        if(getenv("DEBUG_PARSER")) {
            kbdlog(RED "consume" RESET " %s", kbtoken_string(tok->kind));
        }
#endif
    return tok;
}

#if DEBUG
static struct kbtoken * unchecked_eat(struct kbparser* parser, enum kbtoken_kind kind, char* file, int line) {
#else
static struct kbtoken * unchecked_eat(struct kbparser* parser, enum kbtoken_kind kind) {
#endif
    struct kbtoken * tok = NULL;
    if (peek(parser, kind)) {
        tok = &parser->tokens->data[parser->cursor];
#ifdef DEBUG
        if(getenv("DEBUG_PARSER")) {
            kbdlog(RED "consume" RESET " %s", kbtoken_string(kind));
        }
#endif
    }
    else {
        int bufsize = 128;
        char * strbuf = kbmalloc(128 * bufsize);
        int size;
#if DEBUG
    while ((size = snprintf(strbuf, bufsize, "(%s:%d) Unexpected token at %s:%d:%d\nExpected %s but got %s\n", file, line, parser->src->filename, parser->tokens->data[parser->cursor].loc.line, parser->tokens->data[parser->cursor].loc.col, kbtoken_string(kind), kbtoken_string(parser->tokens->data[parser->cursor].kind))) > bufsize) {
#else
    while ((size = snprintf(strbuf, bufsize, "Unexpected token at %s:%d:%d\nExpected %s but got %s\n", parser->src->filename, parser->tokens->data[parser->cursor].loc.line, parser->tokens->data[parser->cursor].loc.col, kbtoken_string(kind), kbtoken_string(parser->tokens->data[parser->cursor].kind))) > bufsize) {
#endif
            bufsize = size+1;
            strbuf = kbrealloc(strbuf, (size+1) * sizeof(char));
        }
        kberrvec_push(&parser->errvec, kberr_make(ESYNTAX, strbuf));
#ifdef DEBUG
    if(getenv("DEBUG_PARSER")) {
        kbdlog(RED "consume" RESET " expected:%s actual:%s", kbtoken_string(kind), kbtoken_string(parser->tokens->data[parser->cursor].kind));
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

static struct kbtoken * unchecked_eatid(struct kbparser* parser, char * id) {
    struct kbtoken * tok = NULL;
    if (peek(parser, TId)) {
        if (!strcmp(parser->tokens->data[parser->cursor].value, id)) {
            tok = &parser->tokens->data[parser->cursor];
#if DEBUG
            if(getenv("DEBUG_PARSER")) {
                kbdlog(RED "consumeid" RESET " ID:%s", id);
            }
#endif
        }
#if DEBUG
        else {
            if(getenv("DEBUG_PARSER")) {
                kbdlog(RED "consumeid" RESET " expected:%s actual:%s", id, parser->tokens->data[parser->cursor].value);
            }
        }
#endif
    }
    if(!tok) {
        int bufsize = 128;
        char * strbuf = kbmalloc(128 * bufsize);
        int size;
        while ((size = snprintf(strbuf, bufsize, "Unexpected identifier at %s:%d:%d\nExpected %s but got %s\n", parser->src->filename, parser->tokens->data[parser->cursor].loc.line, parser->tokens->data[parser->cursor].loc.col, id, parser->tokens->data[parser->cursor].value)) > bufsize) {
            bufsize = size+1;
            strbuf = kbrealloc(strbuf, (size+1) * sizeof(char));
        }
        kberrvec_push(&parser->errvec, kberr_make(ESYNTAX, strbuf));
#ifdef DEBUG
        if(getenv("DEBUG_PARSER")) {
            kbdlog(RED "consumeid" RESET " expected:%s=%s actual:%s", kbtoken_string(TId), id, kbtoken_string(parser->tokens->data[parser->cursor].kind));
        }
#endif
    }

    parser->cursor++;
    return tok;
}

#define eatid(P,K) unchecked_eatid(P, K);errcheck(P)

static int unchecked_make(struct kbparser* parser, int nid) {
    (void)parser;
#if DEBUG
    if (nid != -1 && getenv("DEBUG_PARSER")) {
        kbdlog(RED "<<< make" RESET " %s", kbnode_kind_str(parser->ast->nodes.data[nid].kind));
    }
#endif
    return nid;
}

#define make(P,R) unchecked_make(P,R);errcheck(P)

static void unchecked_push_make(struct kbparser* parser, int group, int recres) {
#if DEBUG
    if(getenv("DEBUG_PARSER")) {
        kbdlog(RED "<<< push_make" RESET " %s", kbnode_kind_str(parser->ast->nodes.data[group].kind));
    }
    assert(isgroup(&parser->ast->nodes.data[group]) );
#endif
    if (recres == -1) return;
    if (attr(parser, group, numitems) == attr(parser, group, itemscap)) {
        attr(parser, group, itemscap) = (attr(parser, group, itemscap) > 0)? attr(parser, group, itemscap) * 2 : 1;
        attr(parser, group, items) = kbrealloc(attr(parser, group, items), sizeof(attr(parser, group, items)[0]) * attr(parser, group, itemscap));
    }
    attr(parser, group, items)[attr(parser, group, numitems)] = recres;
    ++ attr(parser, group, numitems);
}

#define push_make(P,C,R) unchecked_push_make(P,C,R);errcheck(P)

/* Allocate a new ast node */
static int kbparser_addnode(struct kbparser* parser, enum kbnode_kind kind) {
#if DEBUG
    if(getenv("DEBUG_PARSER")) {
        kbdlog(RED ">>> newnode" RESET " %s", kbnode_kind_str(kind));
    }
#endif
    struct kbtoken* token = anypeek(parser);
    int newnode = kbast_add(parser->ast, kind, parser->curnode, token->loc);
    parser->curnode = parser->numnodes;
    return newnode;
}

int is_sym(struct kbtoken* tok) {
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

static int make_todo(struct kbparser* parser) {
    printf("TODO\n");
    kberrvec_push(&parser->errvec, kberr_make(ETODO, "Unimplemented grammar rule"));
    return bubble(parser, -1);
}

static int make_id(struct kbparser* parser) {
    int id = kbparser_addnode(parser, NId);
    struct kbtoken * tokid = eat(parser, TId);
#ifdef DEBUG
    assert(tokid->value);
#endif
    attr(parser, id, name) = kbmalloc((strlen(tokid->value) + 1) * sizeof(char));
    strcpy(attr(parser, id, name), tokid->value);
    return bubble(parser, id);
}

static int make_id_str(struct kbparser* parser, char* str) {
    int id = kbparser_addnode(parser, NId);
    attr(parser, id, name) = kbmalloc((strlen(str) + 1) * sizeof(char));
    strcpy(attr(parser, id, name), str);
    return bubble(parser, id);
}

static int make_sym(struct kbparser* parser) {
    int id = kbparser_addnode(parser, NId);
    struct kbtoken* tok = anypeek(parser);
    if (is_sym(tok)) {
        anyeat(parser);
        attr(parser, id, name) = kbmalloc((strlen(specials[tok->kind]) + 1) * sizeof(attr(parser, id, name)[0]));
        strcpy(attr(parser, id, name), specials[tok->kind]);
    }
    else {
        kbelog("expected sym");
        exit(1);
        return bubble(parser, -1);
    }
    return bubble(parser, id);
}

static int make_type_str(struct kbparser* parser, char* str) {
    int  type = kbparser_addnode(parser, NType);
    attr(parser, type, name) = kbmalloc((strlen(str) + 1) * sizeof(char));
    strcpy(attr(parser, type, name), str);
    return bubble(parser, type);
}

static int make_funparam(struct kbparser* parser) {
    int funparam = kbparser_addnode(parser, NFunParam);

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

static int make_funparam_unit(struct kbparser* parser) {
    int funparam = kbparser_addnode(parser, NFunParam);
    attr(parser, funparam, id) = make(parser, make_id_str(parser, "_"));
    attr(parser, funparam, type) = make(parser, make_type_str(parser, "()"));
    return bubble(parser, funparam);
}

static int make_type(struct kbparser* parser) {
    int  type = kbparser_addnode(parser, NType);
    
    if (peek(parser, TUnit)) {
        eat(parser, TUnit);
        attr(parser, type, name) = kbmalloc((strlen("()") + 1) * sizeof(char));
        strcpy(attr(parser, type, name), "()");
    }
    else {
        struct kbtoken * tokid = eat(parser, TId);
        attr(parser, type, name) = kbmalloc((strlen(tokid->value) + 1) * sizeof(char));
        strcpy(attr(parser, type, name), tokid->value);
    }

    return bubble(parser, type);
}

static int make_strlit(struct kbparser* parser) {
    int strlit = kbparser_addnode(parser, NStrLit);
    struct kbtoken * tok = eat(parser, TStr);
    attr(parser, strlit, value) = kbmalloc(strlen(tok->value) + 1);
    strcpy(attr(parser, strlit, value), tok->value);
    return bubble(parser, strlit);
}

static int make_intlit(struct kbparser* parser) {
    int intlit = kbparser_addnode(parser, NIntLit);
    struct kbtoken * tok = eat(parser, TInt);
    attr(parser, intlit, value) = kbmalloc(strlen(tok->value) + 1);
    strcpy(attr(parser, intlit, value), tok->value);
    return bubble(parser, intlit);
}

static int make_floatlit(struct kbparser* parser) {
    int floatlit = kbparser_addnode(parser, NFloatLit);
    struct kbtoken * tok = eat(parser, TFloat);
    attr(parser, floatlit, value) = kbmalloc(strlen(tok->value) + 1);
    strcpy(attr(parser, floatlit, value), tok->value);
    return bubble(parser, floatlit);
}

static int make_charlit(struct kbparser* parser) {
    int charlit = kbparser_addnode(parser, NCharLit);
    struct kbtoken * tok = eat(parser, TChar);
    attr(parser, charlit, value) = kbmalloc(strlen(tok->value) + 1);
    strcpy(attr(parser, charlit, value), tok->value);
    return bubble(parser, charlit);
}

static int make_lit(struct kbparser* parser) {
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

static int make_term(struct kbparser* parser) {
    struct kbtoken* tok = anypeek(parser);
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

static int make_call(struct kbparser* parser) {
    int call = kbparser_addnode(parser, NCall);
    if (is_builtin_fun(anypeek(parser)->kind)) {
        attr(parser, call, id) = make(parser, make_sym(parser));
    }
    else {
        attr(parser, call, id) = make(parser, make_id(parser));
    }
    attr(parser, call, callparams) = make(parser, make_callparams(parser));
    return bubble(parser, call);
}

static int make_seq(struct kbparser* parser) {
    int seq = kbparser_addnode(parser, NSeq);
    eat(parser, TLineFeed);
    eat(parser, TIndent);

    if (peek(parser, TDedent)) {
        // TODO: handle error
        kbelog("empty sequence");
        exit(1);
    }
    
    do {
        push_make(parser, seq, make_expr(parser));
        if(peek(parser, TLineFeed)) eat(parser, TLineFeed);
    } while(!peek(parser, TDedent));

    eat(parser, TDedent);
    return bubble(parser, seq);
}

static int make_expr(struct kbparser* parser) {
    int expr;
    if (peek(parser, TLineFeed)) {
        expr = make_seq(parser);
    }
    else if (peek(parser, TLPar) || lookahead(parser, 1, TLineFeed) || lookahead(parser, 1, TDedent) || lookaheadid(parser, 1, "then") || lookaheadid(parser, 1, "else") || lookaheadid(parser, 1, "elif") || lookahead(parser, 1, TRPar)) {
        expr = make_term(parser);
    }
    else if (peekid(parser, "if")) {
        expr = make(parser, make_ifelse(parser));
    }
    else if (peekid(parser, "case")) {
        expr = make(parser, make_todo(parser));
    }
    else {
        expr = make(parser, make_call(parser));
    }
    
    return bubble(parser, expr);
}

static int make_callparam(struct kbparser* parser) {
    int callparam = kbparser_addnode(parser, NCallParam);
    attr(parser, callparam, expr) = make(parser, make_term(parser));
    return bubble(parser, callparam);
}

static int make_callparams(struct kbparser* parser) {
    int callparams = kbparser_addnode(parser, NCallParams);
    while(!peek(parser, TLineFeed) && !peekid(parser, "then") && !peekid(parser, "else") && !peek(parser, TRPar) && !peekid(parser, "elif")) {
        push_make(parser, callparams, make_callparam(parser));
    }
    return bubble(parser, callparams);
}

static int make_val(struct kbparser* parser) {
    int expr = kbparser_addnode(parser, NVal);

    if (peekid(parser, "val")) {
        anyeat(parser);
    }

    else if (lookahead(parser, 1, TLineFeed) || lookahead(parser, 1, TDedent)) {
        attr(parser, expr, expr) = make(parser, make_lit(parser));
    }
    else if (peek(parser, TId) && lookahead(parser, 1, TLPar)) {
        attr(parser, expr, expr) = make(parser, make_call(parser));
    }
    else {
        make(parser, make_todo(parser));
    }
    
    if (peek(parser, TLineFeed)) eat(parser, TLineFeed);
    return bubble(parser, expr);
}

static int make_funparams(struct kbparser* parser) {
    int funparams = kbparser_addnode(parser, NFunParams);

    while(!peek(parser, TColonColon) && !peek(parser, TEq) && !peek(parser, TLineFeed)) {
        push_make(parser, funparams, make_funparam(parser));
    }
    return bubble(parser, funparams);
}

static int make_funparams_unit(struct kbparser* parser) {
    int funparams = kbparser_addnode(parser, NFunParams);

    push_make(parser, funparams, make_funparam_unit(parser));
    return bubble(parser, funparams);
}

static int make_fun(struct kbparser* parser) {
    int fun = kbparser_addnode(parser, NFun);
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

static int make_ifelse(struct kbparser* parser) {
    int ifelse = kbparser_addnode(parser, NIfElse);
    {
        int ifbranch = kbparser_addnode(parser, NIfBranch);
        eatid(parser, "if");
        attr(parser, ifbranch, cond) = make(parser, make_expr(parser));
        push_make(parser, ifelse, ifbranch);
        eatid(parser, "then");
        attr(parser, ifbranch, conseq) = make(parser, make_expr(parser));
    }
    
    while(peekid(parser, "elif")) {
        eatid(parser, "elif");
        int ifbranch = kbparser_addnode(parser, NElifBranch);
        attr(parser, ifbranch, cond) = make(parser, make_expr(parser));
        push_make(parser, ifelse, ifbranch);
        eatid(parser, "then");
        attr(parser, ifbranch, conseq) = make(parser, make_expr(parser));
    }
    
    if (peekid(parser, "else")) {
        eatid(parser, "else");
        int ifbranch = kbparser_addnode(parser, NElseBranch);
        attr(parser, ifbranch, cond) = -1;
        push_make(parser, ifelse, ifbranch);
        attr(parser, ifbranch, conseq) = make(parser, make_expr(parser));
    }
    return bubble(parser, ifelse);
}

static int make_program(struct kbparser* parser) {
    int group = kbparser_addnode(parser, NProgram);
    while (!peek(parser, TEndFile)) {
        if (peekid(parser, "fun")) {
            push_make(parser, group, make_fun(parser));
        }
        else {
            push_make(parser, group, make_expr(parser));
        }
        if (peek(parser, TLineFeed)) {
            eat(parser, TLineFeed);
        }
        // else {
        //     struct kbtoken* tok = &parser->tokens[parser->cursor];
        //     int bufsize = 128;
        //     char * strbuf = kbmalloc(128 * bufsize);
        //     int size;
        //     while ((size = snprintf(strbuf, bufsize, "Unexpected token %s at %s:%d:%d\n", kbtoken_string(tok->kind), parser->src->filename, parser->tokens[parser->cursor].loc.line, parser->tokens[parser->cursor].loc.col) > bufsize)) {
        //         bufsize = size+1;
        //         strbuf = kbrealloc(strbuf, (size+1) * sizeof(char));
        //     }
        //     kberrvec_push(&parser->errvec, kberr_make(ESYNTAX, strbuf));
        //     return bubble(parser, -1);
        // }
    }
    eat(parser, TEndFile);
    return bubble(parser, group);
}

void kbparser_new(struct kbparser* parser, struct kbvec_token* tokens, struct kbsrc* src, struct kbast* ast) {
    parser->curnode = -1;
    parser->capacity = 0;
    parser->numnodes = 0;
    parser->cursor = 0;
    parser->tokens = tokens;
    parser->src = src;
    parser->ast = ast;
    parser->errvec = kberrvec_make();
    kbast_new(parser->ast);
}

void kbparser_del(struct kbparser* parser) {
    kberrvec_del(&parser->errvec);
}

void kbparse(struct kbvec_token* tokens, struct kbsrc* src, struct kbast* ast) {
    struct kbparser parser;
    kbparser_new(&parser, tokens, src, ast);
    kbparser_run(&parser);
    kbparser_del(&parser);
}

void kbparser_run(struct kbparser* parser) {
    (void)make_val;
    int group = make_program(parser);
#ifdef DEBUG
    if(getenv("DEBUG_PARSER")) {
        kbdlog(RED "DONE " RESET "%d nodes", parser->numnodes);
    }
#endif
    if (group) {
        kberrvec_display(&parser->errvec);
        exit(1);
    }
}
