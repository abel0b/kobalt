cat << END
#ifndef KLSYNTAX__H
#define KLSYNTAX__H

#include "kobalt/kobalt.h"
#include "kobalt/token.h"

enum kl_node_kind {
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

struct kl_node_group {
    int numitems;
    int itemscap;
    int* items;
};

struct kl_node_fun {
    int id;
    int funparams;
    int rettype;
    int body;
};
        
struct kl_node_type {
    char* name;
};
        
struct kl_node_funparam {
    int id;
    int type;
};

struct kl_node_id {
    char* name;
};

struct kl_node_sym {
    enum kl_token_kind kind;
};

struct kl_node_expr {
    int expr;
};

struct kl_node_intlit {
    char* value;
};

struct kl_node_floatlit {
    char* value;
};

struct kl_node_strlit {
    char* value;
};

struct kl_node_charlit {
    char* value;
};

struct kl_node_callparam {
    int expr;
};

struct kl_node_call {
    int id;
    int callparams;
};

struct kl_node_val {
    int id;
    int expr;
};

struct kl_node_forloop {
    int id;
    int start;
    int end;
    int expr;
};

struct kl_node_ifbranch {
    int cond;
    int conseq;
};

struct kl_node_casebranch {
    int pat;
    int conseq;
};

struct kl_node_import {
    char* path;
};

struct kl_node {
    enum kl_node_kind kind;
    int parent;
    struct kl_loc loc;
    union {
        struct kl_node_group group;
        struct kl_node_fun fun;
        struct kl_node_type type;
        struct kl_node_funparam funparam;
        struct kl_node_expr expr;
        struct kl_node_intlit intlit;
        struct kl_node_floatlit floatlit;
        struct kl_node_strlit strlit;
        struct kl_node_charlit charlit;
        struct kl_node_call call;
        struct kl_node_val val;
        struct kl_node_forloop forloop;
        struct kl_node_callparam callparam;
        struct kl_node_id id;
        struct kl_node_sym sym;
        struct kl_node_ifbranch ifbranch;
        struct kl_node_casebranch casebranch;
        struct kl_node_import import;
    } data;
};

char * kl_node_kind_str(enum kl_node_kind kind);

int isgroup(struct kl_node *node);

#endif
END
