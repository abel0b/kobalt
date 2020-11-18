cat << END
#ifndef KBSYNTAX__H
#define KBSYNTAX__H

#include "kobalt/kobalt.h"
#include "kobalt/token.h"
#include "kobalt/dict.h"

enum kbnode_kind {
END

while read line
do
    line=($line)
    cat << END
    N${line[0]},
END
done < lib/kobalt/syntax.csv

cat << END
};

struct kbnode_group {
    int numitems;
    int itemscap;
    int* items;
};

struct kbnode_fun {
    int id;
    int funparams;
    int rettype;
    int body;
};
        
struct kbnode_type {
    char* name;
};
        
struct kbnode_funparam {
    int id;
    int type;
};

struct kbnode_id {
    char* name;
};

struct kbnode_sym {
    enum kbtoken_kind kind;
};

struct kbnode_expr {
    int expr;
};

struct kbnode_intlit {
    char* value;
};

struct kbnode_floatlit {
    char* value;
};

struct kbnode_strlit {
    char* value;
};

struct kbnode_charlit {
    char* value;
};

struct kbnode_callparam {
    int expr;
};

struct kbnode_call {
    int id;
    int callparams;
};

struct kbnode_ifbranch {
    int cond;
    int conseq;
};

struct kbnode_casebranch {
    int pat;
    int conseq;
};

struct kbnode {
    enum kbnode_kind kind;
    int parent;
    struct kbloc loc;
    union {
        struct kbnode_group group;
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
        struct kbnode_sym sym;
        struct kbnode_ifbranch ifbranch;
        struct kbnode_casebranch casebranch;
    } data;
    struct kbdict* ann;
};

char * kbnode_kind_str(enum kbnode_kind kind);

int isgroup(struct kbnode *node);

#endif
END
