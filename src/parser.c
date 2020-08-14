#include "kobalt/parser.h"
#include "kobalt/memory.h"
#include "kobalt/ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* Get attribute of a given node */
#define attr(P,N,A) (P->ast->nodes[N].data.N.A)

/* Allocate a new ast node */
static int kbparser_addnode(struct kbparser * parser, enum kbnode_kind kind) {
#if DEBUG
    if(getenv("DEBUG_PARSER")) printf("newnode %s\n", kbnode_kind_str(kind));
#endif
    int newnode = kbast_add(parser->ast, kind, parser->curnode);
    parser->curnode = parser->numnodes;
    return newnode;
}

/* Check whether the parser is in an error state */
static int reject(struct kbparser * parser) {
    return parser->errvec.numerrs;
}

/* Returns if the parser is in an error state */
int bubble(struct kbparser * parser, int nid) {
    if (reject(parser)) {
#ifdef DEBUG
        assert(nid == -1);
#endif
        return nid;
    }
    parser->curnode = kbast_getnode(parser->ast, nid)->parent;
    return nid;
}

/* Check whether a token should be ignored */
static int ignore(enum kbtoken_kind tokind) {
    return tokind == TComment;
}

#define errcheck(P) do { if(reject(P)) return bubble(P, -1); } while(0)

#define defrule(R) static int make_##R(struct kbparser * parser)

defrule(todo);
defrule(id);
defrule(type);
defrule(lit);
defrule(call);
defrule(expr);
defrule(callparam);
defrule(callparams);
defrule(decl);
defrule(funbody);
defrule(funparam);
defrule(funparams);

/* Look ahead token stream without consuming */
static struct kbtoken * lookahead(struct kbparser * parser, int stride, enum kbtoken_kind expected_kind) {
    enum kbtoken_kind actual_kind;
    while(ignore(actual_kind = parser->tokens[parser->cursor+stride].kind)) {
        parser->cursor++;
    }
    return (actual_kind == expected_kind)? &parser->tokens[parser->cursor+stride] : NULL;
}

static struct kbtoken * peek(struct kbparser * parser, enum kbtoken_kind expected_kind, ...) {
    return lookahead(parser, 0, expected_kind);
}

static struct kbtoken * peekid(struct kbparser * parser, char * id) {
    struct kbtoken * idtok = lookahead(parser, 0, TId);
    if(idtok && !strcmp(idtok->value, id)) return idtok;
    return NULL;
}

static struct kbtoken * eatany(struct kbparser * parser) {
    return &parser->tokens[parser->cursor++];
}

static struct kbtoken * unchecked_eat(struct kbparser * parser, enum kbtoken_kind kind) {
    struct kbtoken * tok = NULL;
    if (peek(parser, kind)) {
        tok = &parser->tokens[parser->cursor];
#ifdef DEBUG
        if(getenv("DEBUG_PARSER")) fprintf(stderr, "consume %s\n", kbtoken_string(kind));
#endif
    }
    else {
        int bufsize = 128;
        char * strbuf = kbmalloc(128 * bufsize);
        int size;
        while ((size = snprintf(strbuf, bufsize, "Unexpected token at %s:%d:%d\nExpected %s but got %s\n", parser->src->filename, parser->tokens[parser->cursor].line, parser->tokens[parser->cursor].col, kbtoken_string(kind), kbtoken_string(parser->tokens[parser->cursor].kind))) > bufsize) {
            bufsize = size+1;
            strbuf = kbrealloc(strbuf, (size+1) * sizeof(char));
        }
        kberrvec_push(&parser->errvec, kberr_make(ESYNTAX, strbuf));
#ifdef DEBUG
    if(getenv("DEBUG_PARSER")) fprintf(stderr, "consume expected:%s actual:%s\n", kbtoken_string(kind), kbtoken_string(parser->tokens[parser->cursor].kind));
#endif
    }
    parser->cursor++;
    return tok;
}

#define eat(P,K) unchecked_eat(P,K);errcheck(P)

static struct kbtoken * unchecked_eatid(struct kbparser * parser, char * id) {
    struct kbtoken * tok = NULL;
    if (peek(parser, TId)) {
        if (!strcmp(parser->tokens[parser->cursor].value, id)) {
            tok = &parser->tokens[parser->cursor];
#ifdef DEBUG
            if(getenv("DEBUG_PARSER")) fprintf(stderr, "consumeid ID:%s\n", id);
#endif
        }
#ifdef DEBUG
        else {
            if(getenv("DEBUG_PARSER")) fprintf(stderr, "consumeid expected:%s actual:%s\n", id, parser->tokens[parser->cursor].value);
        }
#endif
    }
    if(!tok) {        
        int bufsize = 128;
        char * strbuf = kbmalloc(128 * bufsize);
        int size;
        while ((size = snprintf(strbuf, bufsize, "Unexpected identifier at %s:%d:%d\nExpected %s but got %s\n", parser->src->filename, parser->tokens[parser->cursor].line, parser->tokens[parser->cursor].col, id, parser->tokens[parser->cursor].value)) > bufsize) {
            bufsize = size+1;
            strbuf = kbrealloc(strbuf, (size+1) * sizeof(char));
        }
        kberrvec_push(&parser->errvec, kberr_make(ESYNTAX, strbuf));
#ifdef DEBUG
        if(getenv("DEBUG_PARSER")) fprintf(stderr, "consumeid expected:%s=%s actual:%s\n", kbtoken_string(TId), id, kbtoken_string(parser->tokens[parser->cursor].kind));
#endif
    }

    parser->cursor++;
    return tok;
}

#define eatid(P,K) unchecked_eatid(P, K);errcheck(P)

static int unchecked_make(struct kbparser * parser, int nid) {
    (void)parser;
#if DEBUG
    if (nid != -1 && getenv("DEBUG_PARSER")) printf("make %s\n", kbnode_kind_str(kbast_getnode(parser->ast, nid)->kind));
#endif
    return nid;
}

#define make(P,R) unchecked_make(P,R);errcheck(P)

static void unchecked_push_make(struct kbparser * parser, int group, int recres) {
#if DEBUG
    if(getenv("DEBUG_PARSER")) printf("push_make %s\n", kbnode_kind_str(kbast_getnode(parser->ast, group)->kind));
    assert(isgroup(kbast_getnode(parser->ast, group)));
#endif
    if (recres == -1) return;
    if (attr(parser, group, numitems) == attr(parser, group, itemscap)) {
        attr(parser, group, itemscap) = (attr(parser, group, itemscap) > 0)? attr(parser, group, itemscap)*2 : 1;
        attr(parser, group, items) = kbrealloc(attr(parser, group, items), sizeof(attr(parser, group, items)[0]) * attr(parser, group, itemscap));
    }
    attr(parser, group, items)[attr(parser, group, numitems)] = recres;
    ++ attr(parser, group, numitems);
}

#define push_make(P,C,R) unchecked_push_make(P,C,R);errcheck(P)

static int make_todo(struct kbparser * parser) {
    printf("TODO\n");
    kberrvec_push(&parser->errvec, kberr_make(ETODO, "Unimplemented grammar rule"));
    return bubble(parser, -1);
}

static int make_id(struct kbparser * parser) {
    int id = kbparser_addnode(parser, NId);
    struct kbtoken * tokid = eat(parser, TId);
#ifdef DEBUG
    assert(tokid->value);
#endif
    attr(parser, id, name) = malloc((strlen(tokid->value)+1)*sizeof(char));
    strcpy(attr(parser, id, name), tokid->value);
    return bubble(parser, id);
}

static int make_funparam(struct kbparser * parser) {
    int funparam = kbparser_addnode(parser, NFunParam);
    attr(parser, funparam, id) = make(parser, make_id(parser));
    eat(parser, TColon);
    attr(parser, funparam, type) = make(parser, make_id(parser));
    return bubble(parser, funparam);
}

static int make_type(struct kbparser * parser) {
    int  type = kbparser_addnode(parser, NType);
    attr(parser, type, id) = make(parser, make_id(parser));
    return bubble(parser, type);
}


static int make_strlit(struct kbparser * parser) {
    int strlit = kbparser_addnode(parser, NStrLit);
    struct kbtoken * tok = eat(parser, TStr);
    attr(parser, strlit, value) = kbmalloc(strlen(tok->value)+1);
    strcpy(attr(parser, strlit, value), tok->value);
    return bubble(parser, strlit);
}

static int make_intlit(struct kbparser * parser) {
    int intlit = kbparser_addnode(parser, NIntLit);
    struct kbtoken * tok = eat(parser, TInt);
    attr(parser, intlit, value) = atoi(tok->value);
    return bubble(parser, intlit);
}

static int make_floatlit(struct kbparser * parser) {
    int floatlit = kbparser_addnode(parser, NFloatLit);
    struct kbtoken * tok = eat(parser, TFloat);
    attr(parser, floatlit, value) = atof(tok->value);
    return bubble(parser, floatlit);
}

static int make_charlit(struct kbparser * parser) {
    int charlit = kbparser_addnode(parser, NCharLit);
    struct kbtoken * tok = eat(parser, TChar);
    attr(parser, charlit, value) = tok->value[0];
    return bubble(parser, charlit);
}

static int make_lit(struct kbparser * parser) {
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

static int make_call(struct kbparser * parser) {
    int call = kbparser_addnode(parser, NCall);
    attr(parser, call, id) = make(parser, make_id(parser));
    eat(parser, TLPar);
    // eat(parser, STR);
    attr(parser, call, callparams) = make(parser, make_callparams(parser));
    eat(parser, TRPar);
    return bubble(parser, call);
}

static int make_expr(struct kbparser * parser) {
    int expr = kbparser_addnode(parser, NExpr);
    if (peek(parser, TSemi)) {
        // nop
        make(parser, make_todo(parser));
    }
    else if (lookahead(parser, 1, TSemi) || lookahead(parser, 1, TDedent)) {
        if(peek(parser, TStr)) {
            attr(parser, expr, expr) = make_strlit(parser);
        }
        else if (peek(parser, TInt)) {
            attr(parser, expr, expr) = make_intlit(parser);
        }
        else if (peek(parser, TFloat)) {
            attr(parser, expr, expr) = make_floatlit(parser);
        }
        else if (peek(parser, TChar)) {
            attr(parser, expr, expr) = make_charlit(parser);
        }
        else {
            attr(parser, expr, expr) = make(parser, make_id(parser));
        }
    }
    else if (peek(parser, TId) && lookahead(parser, 1, TLPar)) {
        attr(parser, expr, expr) = make(parser, make_call(parser));
    }
    else {
        make(parser, make_todo(parser));
    }
    
    if (peek(parser, TSemi)) eat(parser, TSemi);
    return bubble(parser, expr);
}

static int make_callparam(struct kbparser * parser) {
    int callparam = kbparser_addnode(parser, NCallParam);
    // eat(parser, STR);
    attr(parser, callparam, expr) = make(parser, make_lit(parser));
    return bubble(parser, callparam);
}

static int make_callparams(struct kbparser * parser) {
    int callparams = kbparser_addnode(parser, NCallParams);
    while(!peek(parser, TRPar)) {
        push_make(parser, callparams, make_callparam(parser));
    }
    return bubble(parser, callparams);
}

static int make_decl(struct kbparser * parser) {
    int expr = kbparser_addnode(parser, NExpr);

    if (peekid(parser, "val")) {
        eatany(parser);
    }

    else if (lookahead(parser, 1, TSemi) || lookahead(parser, 1, TDedent)) {
        attr(parser, expr, expr) = make(parser, make_lit(parser));
    }
    else if (peek(parser, TId) && lookahead(parser, 1, TLPar)) {
        attr(parser, expr, expr) = make(parser, make_call(parser));
    }
    else {
        make(parser, make_todo(parser));
    }
    
    if (peek(parser, TSemi)) eat(parser, TSemi);
    return bubble(parser, expr);
}

static int make_funbody(struct kbparser * parser) {
    int funbody = kbparser_addnode(parser, NFunBody);
    while(!peek(parser, TDedent)) {
        push_make(parser, funbody, make_expr(parser));
    }
    return bubble(parser, funbody);
}

static int make_funparams(struct kbparser * parser) {
    int funparams = kbparser_addnode(parser, NFunParams);
    while(!peek(parser, TRPar)) {
        push_make(parser, funparams, make_funparam(parser));
    }
    return bubble(parser, funparams);
}

static int make_fun(struct kbparser * parser) {
    int fun = kbparser_addnode(parser, NFun);
    eatid(parser, "fun");
    attr(parser, fun, id) = make(parser, make_id(parser));
    eat(parser, TLPar);
    attr(parser, fun, funparams) = make(parser, make_funparams(parser));
    eat(parser, TRPar);
    if (peek(parser, TGT)) {
        eat(parser, TGT);
        attr(parser, fun, rettype) = make(parser, make_type(parser));
    }
    else {
        attr(parser, fun, rettype) = -1; // TODO: return unit type
    }
    eat(parser, TEq);
    eat(parser, TIndent);
    attr(parser, fun, funbody) = make(parser, make_funbody(parser));
    eat(parser, TDedent);
    return bubble(parser, fun);
}

static int make_file(struct kbparser * parser) {
    int group = kbparser_addnode(parser, NFile);
    while (!peek(parser, TEndFile)) {
        if (peekid(parser, "fun")) {
            push_make(parser, group, make_fun(parser));
        }
        else {
            push_make(parser, group, make_todo(parser));
            break;
        }
    }
    eat(parser, TEndFile);
    return bubble(parser, group);
}

void kbparser_new(struct kbparser* parser, struct kbtoken* tokens, struct kbsrc* src, struct kbast* ast) {
    parser->curnode = 0;
    parser->capacity = 0;
    parser->numnodes = 0;
    parser->cursor = 0;
    parser->tokens = tokens;
    parser->src = src;
    parser->ast = ast;
    parser->errvec = kberrvec_make();
    kbast_new(parser->ast);
}

void kbparser_del(struct kbparser * parser) {
    kberrvec_del(&parser->errvec);
}

void kbparse(struct kbtoken* tokens, struct kbsrc* src, struct kbast* ast) {
    struct kbparser parser;
    kbparser_new(&parser, tokens, src, ast);
    kbparser_run(&parser);
    kbparser_del(&parser);
}

void kbparser_run(struct kbparser * parser) {
    int group = make_file(parser);
#ifdef DEBUG
    if(getenv("DEBUG_PARSER")) printf("%d nodes / %d file\n", parser->numnodes, group);
#endif
    if (group) {
        kberrvec_display(&parser->errvec);
        exit(1);
    }
}
