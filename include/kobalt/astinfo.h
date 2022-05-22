#ifndef KLASTINFO__H
#define KLASTINFO__H

#include "abl/dict.h"
#include "abl/vec.h"
#include "abl/objpool.h"
#include "kobalt/type.h"
#include "kobalt/options.h"
#include "kobalt/symbol.h"
#include "kobalt/ast.h"
#include <stdint.h>

struct kl_scope {
    struct abl_dict data;
    struct kl_scope* parent; 
};

abl_vec_decl(struct kl_scope*, scope)

struct kl_astinfo {
    struct abl_objpool type_pool;
    struct abl_objpool scope_pool;
    struct abl_objpool symbol_pool;
    struct abl_vec_scope scopes;
    struct abl_vec_type types;
};

void kl_ast_display(struct kl_opts* opts, FILE* out, struct kl_ast* ast, struct kl_astinfo* astinfo);

struct kl_scope* kl_astinfo_alloc_scope(struct kl_astinfo* astinfo, struct kl_scope* pscope);

void kl_astinfo_new(struct kl_astinfo* astinfo, struct kl_ast* ast);

void kl_astinfo_del(struct kl_astinfo* astinfo);

struct kl_symbol* kl_scope_resolve(struct kl_astinfo* astinfo, char* name, int nid);

struct kl_symbol* kl_scope_try_resolve(struct kl_astinfo* astinfo, char* name, int nid);

struct kl_symbol* kl_scope_define(struct kl_astinfo* astinfo, char* name, int nid);

#endif
