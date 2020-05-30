#include "kobalt/parser.h"
#include "kobalt/memory.h"
#include "kobalt/ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct kbresult {
    int numerrs;
    int numtoks;
    struct kbnode * node;
};

static struct kbnode * nodalloc(struct kbparser * parser) {
    if (parser->numnodes == parser->capacity) {
        parser->buffer = realloc(parser->buffer, 2*parser->capacity);        
    }
    return &parser->buffer[parser->numnodes++];
}

static struct kbnode * mknode(struct kbparser * parser, enum kbnode_kind kind, struct kbnode * parent) {
    struct kbnode * node = nodalloc(parser);
    node->kind = kind;
    node->parent = parent;
    memset(&node->data, 0, sizeof(node->data));
    return node;
}

static struct kbresult mkres(struct kbparser * parser, enum kbnode_kind kind, struct kbnode * parent) {
    struct kbresult res;
    res.numerrs = 0;
    res.numtoks = 0;
    res.node = mknode(parser, kind, parent);
    return res;
}

static bool ignore(enum kbtoken_kind tokind) {
    return tokind == COMMENT;
}

static bool peek(struct kbparser * parser, enum kbtoken_kind expected_kind, struct kbresult * res) {
    enum kbtoken_kind actual_kind;
    while(ignore(actual_kind = parser->tokens[parser->cursor].kind)) {
        parser->cursor++;
        res->numtoks++; 
    }
#ifdef DEBUG
    fprintf(stderr, "lookahead expected:%s actual:%s\n", kbtoken_string(expected_kind), kbtoken_string(actual_kind));
#endif
    return actual_kind == expected_kind;
}

// static bool peekid(struct kbparser * parser, char * id, struct kbresult * res) {
//     enum kbtoken_kind target_kind;
//     while(ignore(target_kind = IDENTIFIER)) {
//         parser->cursor++;
//         res->numtoks++;
//     }
//     if (target_kind == IDENTIFIER) return !strcmp(parser->tokens[parser->cursor].value, id);
//     return false;
// }

static struct kbtoken * eat(struct kbparser * parser, enum kbtoken_kind kind, struct kbresult * res) {
    struct kbtoken * tok = NULL;
    if (peek(parser, kind, res)) {
        tok = &parser->tokens[parser->cursor];
    }
    else {
        int bufsize = 128;
        char * strbuf = kbmalloc(128 * bufsize);
        int size;
        while ((size = snprintf(strbuf, bufsize, "Unexpected token at %s:%d:%d\nExpected %s but got %s\n", parser->src->filename, parser->tokens[parser->cursor].line, parser->tokens[parser->cursor].col, kbtoken_string(kind), kbtoken_string(parser->tokens[parser->cursor].kind))) > bufsize) {
            bufsize = size+1;
            strbuf = kbrealloc(strbuf, bufsize * sizeof(char), (size+1) * sizeof(char));
        }
        kberrvec_push(&parser->errvec, kberr_make(ESYNTAX, strbuf));
        res->numerrs++;
    }
#ifdef DEBUG
    fprintf(stderr, "consume expected:%s actual:%s\n", kbtoken_string(kind), kbtoken_string(parser->tokens[parser->cursor].kind));
#endif
    parser->cursor++;
    res->numtoks++;
    return tok;
}

static struct kbtoken * eatid(struct kbparser * parser, char * id, struct kbresult * res) {
    struct kbtoken * tok = NULL;
    if (peek(parser, IDENTIFIER, res) && !strcmp(parser->tokens[parser->cursor].value, id)) {
        tok = &parser->tokens[parser->cursor];
    }
    else {        
        int bufsize = 128;
        char * strbuf = kbmalloc(128 * bufsize);
        int size;
        while ((size = snprintf(strbuf, bufsize, "Unexpected identifier at %s:%d:%d\nExpected %s but got %s\n", parser->src->filename, parser->tokens[parser->cursor].line, parser->tokens[parser->cursor].col, id, parser->tokens[parser->cursor-1].value)) > bufsize) {
            bufsize = size+1;
            strbuf = kbrealloc(strbuf, bufsize * sizeof(char), (size+1) * sizeof(char));
        }
        kberrvec_push(&parser->errvec, kberr_make(ESYNTAX, strbuf));
        res->numerrs++;
    }
    parser->cursor++;
    res->numtoks++;
    return tok;
}

static void backtrack(struct kbparser * parser, struct kbresult * childres) {
    kberrvec_shrink(&parser->errvec, childres->numerrs);
    parser->cursor -= childres->numtoks;
}

static bool make(struct kbresult * res, struct kbnode ** node, struct kbresult childres) {
    res->numtoks += childres.numtoks;
    res->numerrs += childres.numerrs;
    (*node) = childres.node;
    return !childres.numerrs;
}

static bool try_make_push(struct kbparser * parser, struct kbresult * res, struct kbnode *** items, int * numitems, int * itemscap, struct kbresult childres) {
    if (*numitems == *itemscap) {
        *items = kbrealloc(*items, sizeof(struct kbnode *) * (*itemscap), sizeof(struct kbnode *) * ((*itemscap > 0)? (*itemscap)*2 : 1));
        *itemscap = (*itemscap)*2;
    }
    if (childres.numerrs) {
        backtrack(parser, &childres);
    }
    else {
        res->numtoks += childres.numtoks;
        res->numerrs += childres.numerrs;
        (*items)[*numitems] = childres.node;
        (*numitems)++;
    }
    return !childres.numerrs;
}

static bool make_push(struct kbresult * res, struct kbnode *** items, int * numitems, int * itemscap, struct kbresult childres) {
    if (*numitems == *itemscap) {
        *items = kbrealloc(*items, sizeof(struct kbnode *) * (*itemscap), sizeof(struct kbnode *) * ((*itemscap > 0)? (*itemscap)*2 : 1));
        *itemscap = (*itemscap)*2;
    }
    res->numtoks += childres.numtoks;
    res->numerrs += childres.numerrs;
    if (!childres.numerrs) {
        (*items)[*numitems] = childres.node;
        (*numitems)++;
    }
    return !childres.numerrs;
}

static bool try_make(struct kbparser * parser, struct kbresult * res, struct kbnode ** node, struct kbresult childres) {
    if (childres.numerrs) {
        backtrack(parser, &childres);
    }
    else {
        res->numtoks += childres.numtoks;
        res->numerrs += childres.numerrs;
        (*node) = childres.node;
    }
    return !childres.numerrs;
}

static struct kbresult make_id(struct kbparser * parser, struct kbnode * parent) {
    struct kbresult res = mkres(parser, NID, parent);
    struct kbnode_id * id = &res.node->data.id;
    struct kbtoken * tokid = eat(parser, IDENTIFIER, &res);
    if (res.numerrs) return res;
    id->name = malloc((strlen(tokid->value)+1)*sizeof(char));
    strcpy(id->name, tokid->value); 
    return res;
}

static struct kbresult make_arg(struct kbparser * parser, struct kbnode * parent) {
    struct kbresult res = mkres(parser, NARG, parent);
    struct kbnode_arg * arg = &res.node->data.arg;
    if (!make(&res, &arg->id, make_id(parser, res.node))) return res;
    eat(parser, COLON, &res);
    if (res.numerrs) return res;
    if (!make(&res, &arg->type, make_id(parser, res.node))) return res;
    return res;
}

static struct kbresult make_type(struct kbparser * parser, struct kbnode * parent) {
    struct kbresult res = mkres(parser, NTYPE, parent);
    struct kbnode_type * type = &res.node->data.type;
    if (!make(&res, &type->id, make_id(parser, res.node))) return res;
    return res;
}

static struct kbresult make_lit(struct kbparser * parser, struct kbnode * parent) {
    struct kbresult res = mkres(parser, NTYPE, parent);
    struct kbnode_literal * literal = &res.node->data.literal;
    if (!make(&res, &literal->value, make_id(parser, res.node))) return res;
    return res;
}

static struct kbresult make_funcall(struct kbparser * parser, struct kbnode * parent) {
    struct kbresult res = mkres(parser, NFUNCALL, parent);
    struct kbnode_funcall * funcall = &res.node->data.funcall;
    if (!make(&res, &funcall->id, make_id(parser, res.node))) return res;
    if(!eat(parser, LPAR, &res)) return res;
    if(!eat(parser, STR, &res)) return res;
    if(!eat(parser, RPAR, &res)) return res;
    return res;
}

static struct kbresult make_expr(struct kbparser * parser, struct kbnode * parent) {
    struct kbresult res = mkres(parser, NEXPR, parent);
    struct kbnode_expr * expr = &res.node->data.expr;
    assert(!res.numerrs);

    if (try_make(parser, &res, &expr->expr, make_funcall(parser, res.node))) {
        if(peek(parser, SEMI, &res)) {
            eat(parser, SEMI, &res);
            return res;
        }
    }
    else if (make(&res, &expr->expr, make_lit(parser, res.node))) {
        eat(parser, SEMI, &res);
        return res;
    }
     
    return res;
}

static struct kbresult make_funbody(struct kbparser * parser, struct kbnode * parent) {
    struct kbresult res = mkres(parser, NFUNBODY, parent);
    struct kbnode_funbody * funbody = &res.node->data.funbody;
    if(!make_push(&res, &funbody->exprs, &funbody->numexprs, &funbody->exprscap, make_expr(parser, res.node))) return res;
    return res;
}

static struct kbresult make_fun(struct kbparser * parser, struct kbnode * parent) {
    struct kbresult res = mkres(parser, NFUN, parent);
    struct kbnode_fun * fun = &res.node->data.fun;

    if (!eatid(parser, "fun", &res)) return res;
    
    if (!eat(parser, IDENTIFIER, &res)) return res;
      
    if (!eat(parser, LPAR, &res)) return res;

    while(try_make_push(parser, &res, &fun->args, &fun->numargs, &fun->argscap, make_arg(parser, res.node)));
    
    if (!eat(parser, RPAR, &res)) return res;

    if (!eat(parser, RARROW, &res)) return res;

    if (!make(&res, &fun->rettype, make_type(parser, res.node))) return res;
    
    if (!eat(parser, INDENT, &res)) return res;

    if (!make(&res, &fun->funbody, make_funbody(parser, res.node))) return res;

    if (!eat(parser, DEDENT, &res)) return res;
    
    return res;
}

static struct kbresult make_prog(struct kbparser * parser, struct kbnode * parent) {
    struct kbresult res = mkres(parser, NPROG, parent);
    struct kbnode_prog * prog = &res.node->data.prog;
    while (!peek(parser, END, &res)) {
        if (!make_push(&res, &prog->items, &prog->numitems, &prog->itemscap, make_fun(parser, res.node))) return res;
    }
    if (!eat(parser, END, &res)) return res;
    return res;
}

struct kbparser kbparser_make(struct kbtoken * tokens, struct kbsrc * src) {
    struct kbparser parser = {
        .ast = NULL,
        .capacity = 32,
        .numnodes = 0,
        .cursor = 0,
        .tokens = tokens,
        .src = src,
        .buffer = malloc(32 * sizeof(struct kbnode)),
        .errvec = kberrvec_make(),
    };
    return parser;
}

void kbparser_destroy(struct kbparser * parser) {
    free(parser->buffer);
    kberrvec_destroy(&parser->errvec);
}

void kbparse(struct kbparser * parser) {
    struct kbresult res = make_prog(parser, NULL);
    parser->ast = res.node;
    if (parser->errvec.numerrs) {
#ifdef DEBUG
        kbast_display(parser->ast);
#endif
        kberrvec_display(&parser->errvec);
        assert(!res.numerrs);
        exit(1);
    }
    else {
        printf("Parsing OK\n");
    }
}
