#ifndef KBSYNTAX__H
#define KBSYNTAX__H

enum kbnode_kind {
    NFile,
    NFun,
    NFunParams,
    NFunParam,
    NFunBody,
    NDecl,
    NType,
    NExpr,
    NTerm,
    NBinExpr,
    NStrLit,
    NIntLit,
    NFloatLit,
    NCharLit,
    NCall,
    NCallParams,
    NCallParam,
    NAssign,
    NId,
};

struct kbnode_group {
    int numitems;
    int itemscap;
    int * items;
};

struct kbnode_fun {
    int id;
    int funparams;
    int rettype;
    int funbody;
};
        
struct kbnode_type {
    int id;
};
        
struct kbnode_funparam {
    int id;
    int type;
};

struct kbnode_id {
    char * name;
};

struct kbnode_expr {
    int expr;
};

struct kbnode_intlit {
    int value;
};

struct kbnode_floatlit {
    float value;
};

struct kbnode_strlit {
    char * value;
};

struct kbnode_charlit {
    char value;
};

struct kbnode_callparam {
    int expr;
};

struct kbnode_call {
    int id;
    int callparams;
};

struct kbnode {
    enum kbnode_kind kind;
    int parent;
    union {
        struct kbnode_group group; /* prog, funbody, callparams */
        struct kbnode_fun fun;
        struct kbnode_type type;
        struct kbnode_funparam funparam;
        struct kbnode_expr expr;
        struct kbnode_intlit intlit;
        struct kbnode_floatlit floatlit;
        struct kbnode_strlit strlit;
        struct kbnode_charlit charlit;
        struct kbnode_call call;
        struct kbnode_callparam callparam;
        struct kbnode_id id;
    } data;
};

char * kbnode_kind_str(enum kbnode_kind kind);

int isgroup(struct kbnode *node);

#endif
