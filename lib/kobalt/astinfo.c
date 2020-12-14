#include "kobalt/astinfo.h"
#include <assert.h>

kl_vec_impl(struct kl_scope*, scope)

static void gen_kl_scope_del(void* data) {
    struct kl_scope* scope = (struct kl_scope*)data;
    kl_dict_del(&scope->data);
}

static void gen_kl_type_del(void* data) {
    struct kl_type* type = (struct kl_type*)data;
    kl_type_del(type);
}

static void gen_kl_symbol_del(void* data) {
    struct kl_symbol* symbol = (struct kl_symbol*)data;
    kl_symbol_del(symbol);
}

void kl_astinfo_new(struct kl_astinfo* astinfo, struct kl_ast* ast) {
    kl_objpool_new(&astinfo->type_pool, sizeof(struct kl_type), gen_kl_type_del);
    kl_objpool_new(&astinfo->scope_pool, sizeof(struct kl_scope), gen_kl_scope_del);
    kl_objpool_new(&astinfo->symbol_pool, sizeof(struct kl_symbol), gen_kl_symbol_del);
    kl_vec_scope_new(&astinfo->scopes);
    kl_vec_type_new(&astinfo->types);
    kl_vec_scope_resize(&astinfo->scopes, ast->nodes.size);
    kl_vec_type_resize(&astinfo->types, ast->nodes.size);
    kl_vec_type_fill(&astinfo->types, NULL);
}

void kl_astinfo_del(struct kl_astinfo* astinfo) {
    kl_objpool_del(&astinfo->type_pool);
    kl_objpool_del(&astinfo->scope_pool);
    kl_objpool_del(&astinfo->symbol_pool);
    kl_vec_scope_del(&astinfo->scopes);
    kl_vec_type_del(&astinfo->types);
}

struct kl_scope* kl_astinfo_alloc_scope(struct kl_astinfo* astinfo, struct kl_scope* pscope) {
    struct kl_scope* scope = (struct kl_scope*)kl_objpool_alloc(&astinfo->scope_pool);
    kl_dict_new(&scope->data);
    scope->parent = pscope;
    return scope;
}

struct kl_symbol* kl_scope_resolve(struct kl_astinfo* astinfo, char* name, int nid) {
    struct kl_symbol* symbol = kl_scope_try_resolve(astinfo, name, nid);
    if (!symbol) {
        kl_elog("undefined symbol '%s'", name);
        exit(1);
    }
    return symbol;
}

struct kl_symbol* kl_scope_try_resolve(struct kl_astinfo* astinfo, char* name, int nid) {
    struct kl_scope* scope = astinfo->scopes.data[nid];
    while(scope != NULL) {
        void* symbol = kl_dict_get(&scope->data, name);
        if (symbol) {
            return (struct kl_symbol*)symbol;
        }
        scope = scope->parent;
    }
    return NULL;
}

struct kl_symbol* kl_scope_define(struct kl_astinfo* astinfo, char* name, int nid) {
    struct kl_symbol* symbol = (struct kl_symbol*) kl_objpool_alloc(&astinfo->symbol_pool);
    struct kl_scope* scope = astinfo->scopes.data[nid];
    if (!scope) {
        kl_elog("node '%d' has no defined scope", nid);
        exit(1);
    }
    kl_dict_set(&scope->data, name, (void*) symbol);
    return symbol;
}

struct kl_ast_disp_ctx {
    FILE* out;
    struct kl_astinfo* astinfo;
};

static void indent(FILE* out, int level) {
    for (int ii=0; ii<level; ++ii) {
        fprintf(out, "  ");
    }
}

static int display_aux_color(struct kl_astvisit* astvisit) {
    struct kl_node* node = &astvisit->ast->nodes.data[astvisit->cur.nid];
    struct kl_ast_disp_ctx* ctx = (struct kl_ast_disp_ctx*)astvisit->ctx;

    indent(ctx->out, astvisit->cur.depth);

    fprintf(ctx->out, BWHT "%s" RESET " <%d:%d>", kl_node_kind_str(node->kind), node->loc.line, node->loc.col);

    switch(node->kind) {
        case NType:
            fprintf(ctx->out, " = \"%s\"", node->data.type.name);
            break;
        case NStrLit:
            fprintf(ctx->out, " = " BYEL "\"%s\"" RESET, node->data.strlit.value);
            break;
        case NIntLit:
            fprintf(ctx->out, " = " BYEL "%s" RESET, node->data.intlit.value);
            break;
        case NFloatLit:
            fprintf(ctx->out, " = " BYEL "%s" RESET, node->data.floatlit.value);
            break;
        case NCharLit:
            fprintf(ctx->out, " = " BYEL "\'%s\'" RESET, node->data.charlit.value);
            break;
        case NId:
            fprintf(ctx->out, " = %s", node->data.id.name);
            break;
        case NImport:
            fprintf(ctx->out, " = %s", node->data.import.path);
            break;
        default:
            break;
    }
    if (ctx->astinfo) {
        struct kl_type* type = ctx->astinfo->types.data[astvisit->cur.nid];
        if (type) {
            fprintf(ctx->out, " :: " GRN);
            kl_type_display(ctx->astinfo->types.data[astvisit->cur.nid]);
            fprintf(ctx->out, RESET);
        }
    }
    fprintf(ctx->out, "\n");
    return 1;
}

static int display_aux(struct kl_astvisit* astvisit) {
    struct kl_node* node = &astvisit->ast->nodes.data[astvisit->cur.nid];
    struct kl_ast_disp_ctx* ctx = (struct kl_ast_disp_ctx*)astvisit->ctx;

    indent(ctx->out, astvisit->cur.depth);

    fprintf(ctx->out, "%s" " <%d:%d>", kl_node_kind_str(node->kind), node->loc.line, node->loc.col);

    switch(node->kind) {
        case NType:
            fprintf(ctx->out, " = \"%s\"", node->data.type.name);
            break;
        case NStrLit:
            fprintf(ctx->out, " = " "\"%s\"", node->data.strlit.value);
            break;
        case NIntLit:
            fprintf(ctx->out, " = " "%s", node->data.intlit.value);
            break;
        case NFloatLit:
            fprintf(ctx->out, " = " "%s", node->data.floatlit.value);
            break;
        case NCharLit:
            fprintf(ctx->out, " = " "\'%s\'", node->data.charlit.value);
            break;
        case NId:
            fprintf(ctx->out, " = %s", node->data.id.name);
            break;
        case NImport:
            fprintf(ctx->out, " = %s", node->data.import.path);
            break;
        default:
            break;
    }
    if (ctx->astinfo) {
        struct kl_type* type = ctx->astinfo->types.data[astvisit->cur.nid];
        if (type) {
            fprintf(ctx->out, " :: ");
            kl_type_display(ctx->astinfo->types.data[astvisit->cur.nid]);
        }
    }
    fprintf(ctx->out, "\n");
    return 1;
}

void kl_ast_display(struct kl_opts* opts, FILE* out, struct kl_ast* ast, struct kl_astinfo* astinfo) {
    struct kl_astvisit visdisp;
    struct kl_ast_disp_ctx ctx = {
        .out = out,
        .astinfo = astinfo,
    };
    if (opts->color) {
        kl_astvisit_new(ast, &ctx, display_aux_color, &visdisp, PreOrder);
    }
    else {
        kl_astvisit_new(ast, &ctx, display_aux, &visdisp, PreOrder);
    }
    kl_astvisit_run(&visdisp);
    kl_astvisit_del(&visdisp);
}
