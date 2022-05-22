#include "kobalt/cgen.h"
#include "kobalt/error.h"
#include "kobalt/options.h"
#include "abl/mem.h"
#include "abl/uid.h"
#include "abl/fs.h"
#include "abl/log.h"
#include "kobalt/type.h"
#include "abl/strstack.h"
#include "abl/str.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

struct kl_opts* gopts;

struct kl_cgenctx {
    struct kl_compiland* compiland;
    struct abl_str headerpath;
    FILE* cheader;
    struct abl_str sourcepath;
    struct abl_str tmp;
    FILE* csource;
    struct abl_str h_code;
    struct abl_str src_code;
    struct abl_str main_code;
    struct abl_str* cur_code;
    int valcount;
    struct abl_str_stack vals;
    struct abl_vec tmpvals;
    int indent_level;
    int indent_max;
    char* indent;
};

void emit(FILE* file, char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(file, fmt, args);
    va_end(args);
}

void indent(struct kl_cgenctx* ctx) {
    if(ctx->indent_level == ctx->indent_max) {
        ctx->indent_max *= 2;
        ctx->indent = abl_realloc(ctx->indent, sizeof(ctx->indent[0]) * (ctx->indent_max + 1));
    }
    ctx->indent[ctx->indent_level] = '\t';
    ++ ctx->indent_level;
    ctx->indent[ctx->indent_level] = '\0';
}

void dedent(struct kl_cgenctx* ctx) {
    -- ctx->indent_level;
    ctx->indent[ctx->indent_level] = '\0';
}


char* kl_type_to_c(struct kl_type* type) {
    // TODO: use hash table
    switch (type->kind) {
        case Unit:
            return "void";
        case Int:
            return "int";
        case Char:
            return "char";
        case Float:
            return "float";
        case Str:
            return "char*";
        default:        
            break;
    }
    abl_elog("unimplemented C type for %d", type->kind);
    exit(1);
}

static int cgen_rec(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid);

static void cgen_callparams(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    for(int i = 0; i < node->data.group.numitems; ++i) {
        cgen_rec(ast, modgraph, modid, ctx, node->data.group.items[i]);
    }
}

static void cgen_program(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    indent(ctx);
    for(int i = 0; i < node->data.group.numitems; ++i) {
        cgen_rec(ast, modgraph, modid, ctx, node->data.group.items[i]);
    }
    dedent(ctx);
}

static void cgen_seq(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    for(int i = 0; i < node->data.group.numitems - 1; ++i) {
        cgen_rec(ast, modgraph, modid, ctx, node->data.group.items[i]);
        abl_str_stack_pop(&ctx->vals);
         // TODO: display warning when expresion result is unused
    }
    cgen_rec(ast, modgraph, modid, ctx, node->data.group.items[node->data.group.numitems - 1]);
}

static void cgen_callparam(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    cgen_rec(ast, modgraph, modid, ctx, node->data.callparam.expr);
}

static void cgen_fun(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    if (node->data.fun.body == -1) {
        return;
    }
    
    struct abl_str* prev_code = ctx->cur_code;
    ctx->cur_code = &ctx->src_code;

    char* name = ast->nodes.data[node->data.fun.id].data.id.name;

    struct abl_str name_str;
    abl_str_new(&name_str);
    abl_str_cat(&name_str, name);
    struct kl_type* type = kl_modgraph_resolve(modgraph, modid, nid, &name_str)->type;
    abl_str_del(&name_str);

    abl_str_resize(&ctx->tmp, 0);
	abl_str_catf(&ctx->tmp, "\n%s %s(", kl_type_to_c(type->data.fun.out_type), name);

	int numparams = ast->nodes.data[node->data.fun.funparams].data.group.numitems;
	int firstparam = 1;
	for (int i = 0; i < numparams; ++i) {
        struct kl_type* in_type = abl_vec_type_get(&type->data.fun.in_types, i);
        if (in_type->kind != Unit) {
            char* paramname = ast->nodes.data[ast->nodes.data[ast->nodes.data[node->data.fun.funparams].data.group.items[i]].data.funparam.id].data.id.name;
            if (!firstparam) {
                abl_str_catf(&ctx->tmp, ", ");
                firstparam = 0;
            }
            abl_str_catf(&ctx->tmp, "%s %s", kl_type_to_c(in_type), paramname);
        }
        else if (numparams == 1) {
            abl_str_catf(&ctx->tmp, "void");
        }
	}

	abl_str_catf(&ctx->tmp, ")");
    
    abl_str_catf(ctx->cur_code, "%s\n{\n", ctx->tmp.data);
    abl_str_catf(&ctx->h_code, "%s;\n", ctx->tmp.data);

	cgen_rec(ast, modgraph, modid, ctx, node->data.fun.body);

	if (type->data.fun.out_type->kind != Unit) {
	    char* val = abl_str_stack_pop(&ctx->vals);
	    abl_str_catf(ctx->cur_code, "%sreturn %s;\n", ctx->indent, val);
	}
	abl_str_catf(ctx->cur_code, "}\n");
    ctx->cur_code = prev_code;
}

static void cgen_strlit(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    unused(modgraph);
    unused(modid);
    abl_str_stack_pushf(&ctx->vals, "\"%s\"", node->data.strlit.value);
}

static void cgen_charlit(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    unused(modgraph);
    unused(modid);
    abl_str_stack_pushf(&ctx->vals, "'%s'", node->data.charlit.value);
}

static void cgen_intlit(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    unused(modgraph);
    unused(modid);
    abl_str_stack_pushf(&ctx->vals, "%s", node->data.intlit.value);
}

static void cgen_floatlit(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
    unused(modgraph);
    unused(modid);
	abl_str_stack_pushf(&ctx->vals, "%s", node->data.floatlit.value);
}

static void cgen_id(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid) {
	struct kl_node* node = &ast->nodes.data[nid];
    unused(modgraph);
    unused(modid);
    abl_str_stack_pushf(&ctx->vals, "%s", node->data.id.name);
}

static void cgen_call(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid) {
	struct kl_node* node = &ast->nodes.data[nid];
	cgen_rec(ast, modgraph, modid, ctx, node->data.call.callparams);
	struct kl_node* sym = &ast->nodes.data[node->data.call.id];
	
    char* kl_name = sym->data.id.name;
    struct abl_str name_str;
    abl_str_new(&name_str);
    abl_str_cat(&name_str, kl_name);
    struct kl_type* type = kl_modgraph_resolve(modgraph, modid, nid, &name_str)->type;
    abl_str_del(&name_str);

    abl_str_catf(ctx->cur_code, "%s", ctx->indent);
    int retval;
    if (type->data.fun.out_type->kind != Unit) {
        retval = ctx->valcount++;
        abl_str_catf(ctx->cur_code, "%s val%d = ", kl_type_to_c(type->data.fun.out_type), retval);
    }    

	if (isalpha(kl_name[0]) || kl_name[0] == '_') {
		char *cname;
        if (kl_name[0] == '_' && kl_name[1] == '_') {
            if (strncmp(kl_name, "__c:", strlen("__c:")) == 0) {
                cname = kl_name + strlen("__c:");
            }
            else {
                abl_elog("undefined magic function '%s'", kl_name);
                exit(1);
            }
        }
        else {
        	cname = kl_name;
        }

		abl_str_catf(ctx->cur_code, "%s(", cname);

        // TODO: add ability to pop several values
        struct abl_vec_cstr paramvals;
        abl_vec_cstr_new(&paramvals);
        for(int i = 0; i < ast->nodes.data[node->data.call.callparams].data.group.numitems; ++i) {
            
		    char* val = abl_str_stack_pop(&ctx->vals);
            abl_vec_cstr_push(&paramvals, val);
		    
        }
        for(int i = paramvals.size - 1; i >= 0; -- i) {
            if (i != paramvals.size - 1) {
                abl_str_catf(ctx->cur_code, ", ");
            }
            abl_str_catf(ctx->cur_code, "%s", paramvals.data[i]);
        }
        abl_vec_cstr_del(&paramvals);
		abl_str_catf(ctx->cur_code, ");\n");
	}
	else {
        if (strcmp(sym->data.id.name, "+") == 0 || strcmp(sym->data.id.name, "-") == 0 || strcmp(sym->data.id.name, "<=") == 0) {
            if (ast->nodes.data[node->data.call.callparams].data.group.numitems != 2) {
                abl_elog("expected 2 parameters for binary function '%s'", sym->data.id.name);
                exit(1);
            }
            char* val2 = abl_str_stack_pop(&ctx->vals);
            char* val1 = abl_str_stack_pop(&ctx->vals);

            abl_str_catf(ctx->cur_code, "%s %s %s;\n", val1, sym->data.id.name, val2);
        }
        else {
            abl_elog("undefined builtin function %s", sym->data.id.name);
            exit(1);
        }
	}
    if (type->data.fun.out_type->kind != Unit) {
        abl_str_stack_pushf(&ctx->vals, "val%d", retval);
    }
    else {
        abl_str_stack_push(&ctx->vals, "void");
    }
}

static void cgen_val(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid) {
	struct kl_node* node = &ast->nodes.data[nid];
    char* name = ast->nodes.data[node->data.val.id].data.id.name;
    struct abl_str name_str;
    abl_str_new(&name_str);
    abl_str_cat(&name_str, name);

    cgen_rec(ast, modgraph, modid, ctx, node->data.val.expr);
    char* val = abl_str_stack_pop(&ctx->vals);

    struct kl_type* type = kl_modgraph_resolve(modgraph, modid, nid, &name_str)->type;
    abl_str_del(&name_str);

    abl_str_catf(ctx->cur_code, "%s%s %s = %s;\n", ctx->indent, kl_type_to_c(type), name, val);

    abl_str_stack_pushf(&ctx->vals, "%s", name);
}

static void cgen_forloop(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid) {
	struct kl_node* node = &ast->nodes.data[nid];
    char* name = ast->nodes.data[node->data.forloop.id].data.id.name;
    struct abl_str name_str;
    abl_str_new(&name_str);
    abl_str_cat(&name_str, name);

    struct kl_type* type = kl_modgraph_resolve(modgraph, modid, nid, &name_str)->type;

    abl_str_catf(ctx->cur_code, "%sfor(%s %s = %d; %s < %d; ++ %s)\n", ctx->indent, kl_type_to_c(type), name, atoi(ast->nodes.data[node->data.forloop.start].data.intlit.value), name, atoi(ast->nodes.data[node->data.forloop.end].data.intlit.value), name);
    abl_str_catf(ctx->cur_code, "%s{\n", ctx->indent);
    indent(ctx);
    cgen_rec(ast, modgraph, modid, ctx, node->data.forloop.expr);
    dedent(ctx);
    abl_str_catf(ctx->cur_code, "%s}\n", ctx->indent);

    char* val = abl_str_stack_pop(&ctx->vals);
    unused(val);
    abl_str_del(&name_str);
    abl_str_stack_push(&ctx->vals, "void");
}


static void cgen_ifelse(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid) {
	struct kl_node* node = &ast->nodes.data[nid];
    int val = ctx->valcount++;
    abl_str_catf(ctx->cur_code, "%s%s val%d;\n", ctx->indent, "int", val);
    abl_vec_push(&ctx->tmpvals, &val);
    for(int i = 0; i < node->data.group.numitems; ++i) {
        cgen_rec(ast, modgraph, modid, ctx, node->data.group.items[i]);
    }
    abl_vec_pop(&ctx->tmpvals, NULL);
    abl_str_stack_pushf(&ctx->vals, "val%d", val);
}

static void cgen_ifbranch(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid) {
	struct kl_node* node = &ast->nodes.data[nid];
	int retval = *(int*)abl_vec_last(&ctx->tmpvals);
	cgen_rec(ast, modgraph, modid, ctx, node->data.ifbranch.cond);
	char *condval = abl_str_stack_pop(&ctx->vals);
	if (node->kind == NElifBranch) {
		abl_str_catf(ctx->cur_code, "%selse\n%s{\n", ctx->indent, ctx->indent);
		indent(ctx);
	}
	abl_str_catf(ctx->cur_code, "%sif (%s)\n\t{\n", ctx->indent, condval);
	indent(ctx);

	cgen_rec(ast, modgraph, modid, ctx, node->data.ifbranch.conseq);
	char *branchval = abl_str_stack_pop(&ctx->vals);
	abl_str_catf(ctx->cur_code, "%sval%d = %s;\n", ctx->indent, retval, branchval);
	dedent(ctx);
	abl_str_catf(ctx->cur_code, "%s}\n", ctx->indent);
	if (node->kind == NElifBranch) {
		dedent(ctx);
		abl_str_catf(ctx->cur_code, "%s}\n", ctx->indent);
	}
}

static void cgen_elsebranch(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid) {
	struct kl_node* node = &ast->nodes.data[nid];
    int retval = *(int*)abl_vec_last(&ctx->tmpvals);
    abl_str_catf(ctx->cur_code, "%selse\n%s{\n", ctx->indent, ctx->indent);
    indent(ctx);
    cgen_rec(ast, modgraph, modid, ctx, node->data.ifbranch.conseq);
    char* branchval = abl_str_stack_pop(&ctx->vals);
    abl_str_catf(ctx->cur_code, "%sval%d = %s;\n", ctx->indent, retval, branchval);
    dedent(ctx);
    abl_str_catf(ctx->cur_code, "%s}\n", ctx->indent);
}

static int cgen_rec(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, struct kl_cgenctx* ctx, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];
#if DEBUG
    if(getenv("DEBUG_CGEN")) {
        abl_dlog("cgen %s", kl_node_kind_str(node->kind));
    }
#endif
    switch(node->kind) {
        case NProgram:
            cgen_program(ast, modgraph, modid, ctx, nid);
            break;
        case NCallParams:
            cgen_callparams(ast, modgraph, modid, ctx, nid);
            break;
        case NSeq:
            cgen_seq(ast, modgraph, modid, ctx, nid);
            break;
        case NCallParam:
            cgen_callparam(ast, modgraph, modid, ctx, nid);
            break;
        case NFun:
            cgen_fun(ast, modgraph, modid, ctx, nid);
            break;
        case NStrLit:
            cgen_strlit(ast, modgraph, modid, ctx, nid);
            break;
        case NIntLit:
            cgen_intlit(ast, modgraph, modid, ctx, nid);
            break;
        case NFloatLit:
            cgen_floatlit(ast, modgraph, modid, ctx, nid);
            break;
        case NCharLit:
            cgen_charlit(ast, modgraph, modid, ctx, nid);
            break;
        case NId:
			cgen_id(ast, modgraph, modid, ctx, nid);
            break;
        case NCall:
			cgen_call(ast, modgraph, modid, ctx, nid);
            break;
        case NVal:
			cgen_val(ast, modgraph, modid, ctx, nid);
            break;
        case NForLoop:
			cgen_forloop(ast, modgraph, modid, ctx, nid);
            break;
        case NIfElse:
			cgen_ifelse(ast, modgraph, modid, ctx, nid);
            break;
        case NIfBranch:
        case NElifBranch:
			cgen_ifbranch(ast, modgraph, modid, ctx, nid);
            break;
        case NElseBranch:
			cgen_elsebranch(ast, modgraph, modid, ctx, nid);
            break;
        default:
            break;
    }
    return 1;
}

void kl_cgenctx_new(struct kl_opts* opts, struct kl_compiland* compiland, struct kl_cgenctx* cgenctx) {
    abl_str_new(&cgenctx->h_code);
    abl_str_new(&cgenctx->tmp);
    abl_str_new(&cgenctx->src_code);
    abl_str_new(&cgenctx->main_code);
    cgenctx->cur_code = &cgenctx->main_code;
    cgenctx->compiland = compiland;
    
    abl_str_new(&cgenctx->headerpath);
    abl_str_new(&cgenctx->sourcepath);
    abl_str_catf(&cgenctx->headerpath, "%s/%s.h", opts->cachepath.data, compiland->name);
    abl_str_catf(&cgenctx->sourcepath, "%s/%s.c", opts->cachepath.data, compiland->name);
    
    cgenctx->cheader = fopen(cgenctx->headerpath.data, "w");
    if (cgenctx->cheader == NULL) {
        abl_elog("couldn't open output file '%s'", cgenctx->headerpath.data);
        exit(1);
    }
   
    cgenctx->csource = fopen(cgenctx->sourcepath.data, "w");
    if (cgenctx->csource == NULL) {
        abl_elog("couldn't open file output '%s'", cgenctx->sourcepath.data);
        exit(1);
    }
    cgenctx->valcount = 1;
    cgenctx->indent_level = 0;
    cgenctx->indent_max = 1;
    cgenctx->indent = abl_malloc(sizeof(cgenctx->indent[0]) * (cgenctx->indent_max + 1));
    cgenctx->indent[0] = '\0';
    abl_str_stack_new(&cgenctx->vals);
    abl_vec_new(&cgenctx->tmpvals, sizeof(int));
}

void kl_cgenctx_del(struct kl_cgenctx* cgenctx) {
    abl_str_del(&cgenctx->tmp);
    abl_str_del(&cgenctx->h_code);
    abl_str_del(&cgenctx->src_code);
    abl_str_del(&cgenctx->main_code);
    abl_str_del(&cgenctx->headerpath);
    abl_str_del(&cgenctx->sourcepath);
    fclose(cgenctx->cheader);
    fclose(cgenctx->csource);
    abl_str_stack_del(&cgenctx->vals);
    abl_vec_del(&cgenctx->tmpvals);
    abl_free(cgenctx->indent);
}

int kl_cgen(struct kl_opts* opts, struct kl_compiland* compiland, struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid) {
    gopts = opts;

    struct kl_cgenctx cgenctx;
    kl_cgenctx_new(opts, compiland, &cgenctx);
    struct kl_mod* mod = kl_modgraph_get(modgraph, modid);
    
    struct abl_str common_header;
    abl_str_new(&common_header);
    abl_str_catf(&common_header, "// This file was generated by Kobalt %s\n", KLVERSION);

    struct abl_str cid;
    abl_str_new(&cid);
    abl_str_cat(&cid, compiland->basename.data);
    for(int i = 0; i < cid.len; ++i) {
        if (!isalpha(cid.data[i]) && !isdigit(cid.data[i])) {
            cid.data[i] = '_';
        }
    }
    
    char* headername = abl_malloc(sizeof(char) * (compiland->name.len + 2 + 8 + 1));
    memset(headername, 0, sizeof(char) * (compiland->name.len + 2 + 8 + 1));
    {
        int cur = 0;
        for(int i = 0; i < compiland->name.len; ++i) {
            if (isalpha(compiland->name.data[i])) {
                headername[cur++] = toupper(compiland->name.data[i]);
            }
            else {
                headername[cur++] = '_';
            }
        }
        headername[cur++] = '_';
        headername[cur++] = '_';
        abl_uid_gen(headername + cur, ABL_UID_UALPHANUM);
    }

    emit(cgenctx.cheader,
        "%s"
        "#ifndef __KL_HEADER_%s\n"
        "#define __KL_HEADER_%s\n"
        "#include <stdlib.h>\n"
        "#include <stdio.h>\n"
        "#include <stdbool.h>\n"
        "#include <stdint.h>\n"
        "#include <math.h>\n",
        common_header.data,
        headername,
        headername
    );

    for(int i = 0; i < mod->deps.size; ++ i) {
        emit(cgenctx.cheader, "#include \"%s.h\"\n", mod->deps.data[i]);
    }

    emit(cgenctx.csource,
        "%s"
        "#include \"%s.h\"\n",
        common_header.data,
        compiland->name.data
    );

   
    cgen_rec(ast, modgraph, modid, &cgenctx, 0);

    if (cgenctx.h_code.len) {
        emit(cgenctx.cheader, cgenctx.h_code.data);
    }

    if (cgenctx.src_code.len) {
        emit(cgenctx.csource, "%s", cgenctx.src_code.data);
    }

    if (cgenctx.main_code.len || compiland->entry) {
        emit(cgenctx.csource,
            "\nint __main__%s()\n"
            "{\n"
            "%s"
            "\treturn 0;\n"
            "}\n",
            cid.data,
            cgenctx.main_code.data
        );
    }

    if (compiland->entry) {
        emit(cgenctx.csource,
            "\nint main()\n"
            "{\n"
            "\tint status = __main__%s();\n"
            "\treturn status;\n"
            "}\n",
            cid.data
        );
    }

    emit(cgenctx.cheader, "#endif\n");
    
    fflush(cgenctx.cheader);
    fflush(cgenctx.csource);
        
    // TODO: create a file copy function in fs.c
    // FILE* srcexe = fopen(exefull, "rb");
    // if (srcexe == NULL) {
    //     abl_elog("could not open file '%s'", exefull);
    //     exit(1);
    // }
    // FILE* destexe = fopen(opts->output, "wb");
    // if (destexe == NULL) {
    //     abl_elog("could not open file '%s'", opts->output);
    //     exit(1);
    // }
    // char ch;
    // while ((ch = fgetc(srcexe)) != EOF) {
    //     fputc(ch, destexe);
    // }
    // fflush(destexe);
    // fclose(srcexe);
    // fclose(destexe);
    // abl_free(exe);
    abl_str_del(&cid);
    kl_cgenctx_del(&cgenctx);
    abl_str_del(&common_header);
    abl_free(headername);
    return 1;
}
