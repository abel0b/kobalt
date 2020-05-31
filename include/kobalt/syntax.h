#ifndef KBSYNTAX__H
#define KBSYNTAX__H

enum kbnode_kind {
    NPROG,
    NITEM,
    NFUN,
    NARG,
    NFUNBODY,
    NDECL,
    NTYPE,
    NEXPR,
    NTERM,
    NBINEXPR,
    NLIT,
    NFUNCALL,
    NASSIGN,
    NID,
};

struct kbnode_prog {
    int numitems;
    int itemscap;
    struct kbnode ** items;
};

struct kbnode_fun {
    struct kbnode * id;
    int numargs;
    int argscap;
    struct kbnode ** args;
    struct kbnode * rettype;
    struct kbnode * funbody;
};
        
struct kbnode_type {
    struct kbnode * id;
};
        
struct kbnode_arg {
    struct kbnode * id;
    struct kbnode * type;
};
    
struct kbnode_funbody {
    int numexprs;
    int exprscap;
    struct kbnode ** exprs;
};

struct kbnode_id {
    char * name;
};

struct kbnode_expr {
    struct kbnode * expr;
};

struct kbnode_literal {
    struct kbnode * value;
};

struct kbnode_funcall {
    struct kbnode * id;
    struct kbnode * arg;
};

struct kbnode {
    enum kbnode_kind kind;
    struct kbnode * parent;
    union {
        struct kbnode_prog prog;
        struct kbnode_fun fun;
        struct kbnode_type type;
        struct kbnode_arg arg;
        struct kbnode_expr expr;
        struct kbnode_literal literal;
        struct kbnode_funcall funcall;
        struct kbnode_funbody funbody;
        struct kbnode_id id;
    } data;
};

#endif
