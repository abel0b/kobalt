#include "kobalt/cgen.h"
#include "kobalt/error.h"
#include "kobalt/options.h"
#include "kobalt/memory.h"
#include "kobalt/uid.h"
#include "kobalt/cmdcc.h"
#include "kobalt/fs.h"
#include "kobalt/log.h"
#include "kobalt/type.h"
#include "kobalt/strpool.h"
#include "kobalt/str.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

bool is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

struct kbcgenctx {
    struct kbsrc* src;
    char* headerpath;
    FILE* cheader;
    char* sourcepath;
    char* include;
    FILE* csource;
    struct kbstr h_code;
    struct kbstr src_code;
    struct kbstr main_code;
    struct kbstr* cur_code;
    int valcount;
    struct kbstrpool vals;
    struct kbvec tmpvals;
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

void indent(struct kbcgenctx* ctx) {
    if(ctx->indent_level == ctx->indent_max) {
        ctx->indent_max *= 2;
        ctx->indent = kbrealloc(ctx->indent, sizeof(ctx->indent[0]) * (ctx->indent_max + 1));
    }
    ctx->indent[ctx->indent_level] = '\t';
    ++ ctx->indent_level;
    ctx->indent[ctx->indent_level] = '\0';
}

void dedent(struct kbcgenctx* ctx) {
    -- ctx->indent_level;
    ctx->indent[ctx->indent_level] = '\0';
}

static int cgen_rec(struct kbast* ast, struct kbastinfo* astinfo, struct kbcgenctx* ctx, int nid);

static void cgen_callparams(struct kbast* ast, struct kbastinfo* astinfo, struct kbcgenctx* ctx, int nid) {
    struct kbnode* node = &ast->nodes[nid];
    for(int i = 0; i < node->data.group.numitems; ++i) {
        cgen_rec(ast, astinfo, ctx, node->data.group.items[i]);
    }
}

static void cgen_file(struct kbast* ast, struct kbastinfo* astinfo, struct kbcgenctx* ctx, int nid) {
    struct kbnode* node = &ast->nodes[nid];
    indent(ctx);
    for(int i = 0; i < node->data.group.numitems; ++i) {
        cgen_rec(ast, astinfo, ctx, node->data.group.items[i]);
    }
    dedent(ctx);
    // if (ctx->vals.lens.num_elems > 0) {
    //     kbelog("unexpected state");
    //     kbelog("%d elements expected in vals objpool, got %d", 0, ctx->vals.lens.num_elems);
    //     exit(1);
    // }
}

static void cgen_seq(struct kbast* ast, struct kbastinfo* astinfo, struct kbcgenctx* ctx, int nid) {
    struct kbnode* node = &ast->nodes[nid];
    for(int i = 0; i < node->data.group.numitems - 1; ++i) {
        cgen_rec(ast, astinfo, ctx, node->data.group.items[i]);
        kbstrpool_pop(&ctx->vals);
         // TODO: display warning when expresion result is unused
    }
    cgen_rec(ast, astinfo, ctx, node->data.group.items[node->data.group.numitems - 1]);
}

static void cgen_callparam(struct kbast* ast, struct kbastinfo* astinfo, struct kbcgenctx* ctx, int nid) {
    struct kbnode* node = &ast->nodes[nid];
    cgen_rec(ast, astinfo, ctx, node->data.callparam.expr);
}

static void cgen_fun(struct kbast* ast, struct kbastinfo* astinfo, struct kbcgenctx* ctx, int nid) {
    struct kbstr* prev_code = ctx->cur_code;
    ctx->cur_code = &ctx->src_code;

    struct kbnode* node = &ast->nodes[nid];
    char* name = ast->nodes[node->data.fun.id].data.id.name;

    struct kbtype* type = (struct kbtype*)kbscope_resolve(astinfo, (uintptr_t)name, nid);
	{
		char* out_type_str;
		switch (type->data.fun.out_type->kind) {
			case Unit:
			out_type_str = "void";
			break;
			case Int:
			out_type_str = "int";
			break;
			default:
			todo();  // TODO: handle other type cases
			break;
		}
		kbstr_catf(ctx->cur_code, "\n%s %s(", out_type_str, name);
	}

	int numparams = ast->nodes[node->data.fun.funparams].data.group.numitems;
	int firstparam = 1;
	for (int i = 0; i < numparams; ++i) {
        struct kbtype* in_type = *(struct kbtype**)kbvec_get(&type->data.fun.in_types, i);
        if (in_type->kind != Unit) {
            char* paramname = ast->nodes[ast->nodes[ast->nodes[node->data.fun.funparams].data.group.items[i]].data.funparam.id].data.id.name;
            char* in_type_str;
            switch (in_type->kind) {
            case Unit:
                in_type_str = "void";
                break;
            case Int:
                in_type_str = "int";
                break;
            default:
                printf("%d\n", in_type->kind);
                todo();	 // TODO: handle other type cases
                break;
            }
            if (!firstparam) {
                kbstr_catf(ctx->cur_code, ", ");
                firstparam = 0;
            }
            kbstr_catf(ctx->cur_code, "%s %s", in_type_str, paramname);
        }
        else if (numparams == 1) {
            kbstr_catf(ctx->cur_code, "void");
        }
	}

	kbstr_catf(ctx->cur_code, ")\n{\n");

	cgen_rec(ast, astinfo, ctx, node->data.fun.funbody);

	if (type->data.fun.out_type->kind != Unit) {
	    char* val = kbstrpool_pop(&ctx->vals);
	    kbstr_catf(ctx->cur_code, "%sreturn %s;\n", ctx->indent, val);
	}
	kbstr_catf(ctx->cur_code, "}\n");
    ctx->cur_code = prev_code;
}

static void cgen_strlit(struct kbast* ast, struct kbastinfo* astinfo, struct kbcgenctx* ctx, int nid) {
    struct kbnode* node = &ast->nodes[nid];
    kbstrpool_push(&ctx->vals, "\"%s\"", node->data.strlit.value);
}

static void cgen_charlit(struct kbast* ast, struct kbastinfo* astinfo, struct kbcgenctx* ctx, int nid) {
    struct kbnode* node = &ast->nodes[nid];
    kbstrpool_push(&ctx->vals, "%s", node->data.charlit.value);
}

static void cgen_intlit(struct kbast* ast, struct kbastinfo* astinfo, struct kbcgenctx* ctx, int nid) {
    struct kbnode* node = &ast->nodes[nid];
    kbstrpool_push(&ctx->vals, "%s", node->data.intlit.value);
}

static void cgen_floatlit(struct kbast* ast, struct kbastinfo* astinfo, struct kbcgenctx* ctx, int nid) {
    struct kbnode* node = &ast->nodes[nid];
	kbstrpool_push(&ctx->vals, "%s", node->data.floatlit.value);
}

static void cgen_id(struct kbast* ast, struct kbastinfo* astinfo, struct kbcgenctx* ctx, int nid) {
	struct kbnode* node = &ast->nodes[nid];
    kbstrpool_push(&ctx->vals, "%s", node->data.id.name);
}

static void cgen_call(struct kbast* ast, struct kbastinfo* astinfo, struct kbcgenctx* ctx, int nid) {
	struct kbnode* node = &ast->nodes[nid];
	cgen_rec(ast, astinfo, ctx, node->data.call.callparams);
	struct kbnode* sym = &ast->nodes[node->data.call.id];
	
    char *kbname = sym->data.id.name;
	int retval = ctx->valcount++;

    kbstr_catf(ctx->cur_code, "%s%s val%d = ", ctx->indent, "int", retval);

	if (is_letter(kbname[0])) {
		char *cname;
		if (strcmp(kbname, "print") == 0) {
			cname = "printf";
		}
		else {
			kbelog("undefined function %s", kbname);
			// exit(1);
			cname = kbname;
		}

		if (strcmp(kbname, "print") == 0) {
			kbstr_catf(ctx->cur_code, "%s(\"%%s\", ", cname);
		}
        else {
			kbstr_catf(ctx->cur_code, "%s(", cname);
		}

        for(int i = 0; i < ast->nodes[node->data.call.callparams].data.group.numitems; ++i) {
            if (i != 0) {
                kbstr_catf(ctx->cur_code, ", ");
            }
		    char* val = kbstrpool_pop(&ctx->vals);
		    kbstr_catf(ctx->cur_code, "%s", val);
        }
		kbstr_catf(ctx->cur_code, ");\n");
	}
	else {
        if (strcmp(sym->data.id.name, "+") == 0 || strcmp(sym->data.id.name, "-") == 0 || strcmp(sym->data.id.name, "<=") == 0) {
            if (ast->nodes[node->data.call.callparams].data.group.numitems != 2) {
                kbelog("expected 2 parameters for binary function '%s'", sym->data.id.name);
                exit(1);
            }
            char* val2 = kbstrpool_pop(&ctx->vals);
            char* val1 = kbstrpool_pop(&ctx->vals);

            kbstr_catf(ctx->cur_code, "%s %s %s;\n", val1, sym->data.id.name, val2);
        }
        else {
            kbelog("undefined builtin function %s", sym->data.id.name);
            exit(1);
        }
	}
    kbstrpool_push(&ctx->vals, "val%d", retval);
}

static void cgen_ifelse(struct kbast* ast, struct kbastinfo* astinfo, struct kbcgenctx* ctx, int nid) {
	struct kbnode* node = &ast->nodes[nid];
    int val = ctx->valcount++;
    kbstr_catf(ctx->cur_code, "%s%s val%d;\n", ctx->indent, "int", val);
    kbvec_push(&ctx->tmpvals, &val);
    for(int i = 0; i < node->data.group.numitems; ++i) {
        cgen_rec(ast, astinfo, ctx, node->data.group.items[i]);
    }
    kbvec_pop(&ctx->tmpvals, NULL);
    kbstrpool_push(&ctx->vals, "val%d", val);
}

static void cgen_ifbranch(struct kbast* ast, struct kbastinfo* astinfo, struct kbcgenctx* ctx, int nid) {
	struct kbnode* node = &ast->nodes[nid];
	int retval;
	kbvec_peek(&ctx->tmpvals, &retval);
	cgen_rec(ast, astinfo, ctx, node->data.ifbranch.cond);
	char *condval = kbstrpool_pop(&ctx->vals);
	if (node->kind == NElifBranch) {
		kbstr_catf(ctx->cur_code, "%selse\n%s{\n", ctx->indent, ctx->indent);
		indent(ctx);
	}
	kbstr_catf(ctx->cur_code, "%sif (%s)\n\t{\n", ctx->indent, condval);
	indent(ctx);

	cgen_rec(ast, astinfo, ctx, node->data.ifbranch.conseq);
	char *branchval = kbstrpool_pop(&ctx->vals);
	kbstr_catf(ctx->cur_code, "%sval%d = %s;\n", ctx->indent, retval, branchval);
	dedent(ctx);
	kbstr_catf(ctx->cur_code, "%s}\n", ctx->indent);
	if (node->kind == NElifBranch) {
		dedent(ctx);
		kbstr_catf(ctx->cur_code, "%s}\n", ctx->indent);
	}
}

static void cgen_elsebranch(struct kbast* ast, struct kbastinfo* astinfo, struct kbcgenctx* ctx, int nid) {
	struct kbnode* node = &ast->nodes[nid];
    int retval;
    kbvec_peek(&ctx->tmpvals, &retval);
    kbstr_catf(ctx->cur_code, "%selse\n%s{\n", ctx->indent, ctx->indent);
    indent(ctx);
    cgen_rec(ast, astinfo, ctx, node->data.ifbranch.conseq);
    char* branchval = kbstrpool_pop(&ctx->vals);
    kbstr_catf(ctx->cur_code, "%sval%d = %s;\n", ctx->indent, retval, branchval);
    dedent(ctx);
    kbstr_catf(ctx->cur_code, "%s}\n", ctx->indent);
}

static int cgen_rec(struct kbast* ast, struct kbastinfo* astinfo, struct kbcgenctx* ctx, int nid) {
    struct kbnode* node = &ast->nodes[nid];

#if DEBUG
	// TODO: cgen debug mode
    kbdlog("cgen %s valcount=%d", kbnode_kind_str(node->kind), ctx->valcount);
#endif
    switch(node->kind) {
        case NFile:
            cgen_file(ast, astinfo, ctx, nid);
            break;
        case NCallParams:
            cgen_callparams(ast, astinfo, ctx, nid);
            break;
        case NSeq:
            cgen_seq(ast, astinfo, ctx, nid);
            break;
        case NCallParam:
            cgen_callparam(ast, astinfo, ctx, nid);
            break;
        case NFun:
            cgen_fun(ast, astinfo, ctx, nid);
            break;
        case NStrLit:
            cgen_strlit(ast, astinfo, ctx, nid);
            break;
        case NIntLit:
            cgen_intlit(ast, astinfo, ctx, nid);
            break;
        case NFloatLit:
            cgen_floatlit(ast, astinfo, ctx, nid);
            break;
        case NCharLit:
            cgen_charlit(ast, astinfo, ctx, nid);
            break;
        case NId:
			cgen_id(ast, astinfo, ctx, nid);
            break;
        case NCall:
			cgen_call(ast, astinfo, ctx, nid);
            break;
        case NIfElse:
			cgen_ifelse(ast, astinfo, ctx, nid);
            break;
        case NIfBranch:
        case NElifBranch:
			cgen_ifbranch(ast, astinfo, ctx, nid);
            break;
        case NElseBranch:
			cgen_elsebranch(ast, astinfo, ctx, nid);
            break;
        default:
            break;
    }
    return 1;
}

void kbcgenctx_new(struct kbopts* opts, struct kbsrc* src, struct kbcgenctx* cgenctx) {
    kbstr_new(&cgenctx->h_code);
    kbstr_new(&cgenctx->src_code);
    kbstr_new(&cgenctx->main_code);
    cgenctx->cur_code = &cgenctx->main_code;
    cgenctx->src = src;
    int lencachedir = strlen(opts->cachedir);
    int srcsize = lencachedir + 1 + strlen(src->filename);
    
    cgenctx->headerpath = kbmalloc(sizeof(char) * srcsize);
    cgenctx->sourcepath = kbmalloc(sizeof(char) * srcsize);
    
    sprintf(cgenctx->headerpath, "%s/", opts->cachedir);
    int i;
    cgenctx->include = &cgenctx->headerpath[lencachedir + 1];
    for(i = 0; i<(int)strlen(src->filename)-3; i++) {
        cgenctx->headerpath[lencachedir + 1 + i] = (isds(src->filename[i]))? '%' : src->filename[i];
    }
    cgenctx->headerpath[lencachedir + 1 + i] = '.';
    cgenctx->headerpath[lencachedir + 1 + i + 1] = 'h';
    cgenctx->headerpath[lencachedir + 1 + i + 2] = '\0';
    cgenctx->cheader = fopen(cgenctx->headerpath, "w");
    if (cgenctx->cheader == NULL) {
        kbelog("couldn't open output file '%s'", cgenctx->headerpath);
        exit(1);
    }
   
    strcpy(cgenctx->sourcepath, cgenctx->headerpath);
    cgenctx->sourcepath[lencachedir + 1 + i + 1] = 'c';
    cgenctx->csource = fopen(cgenctx->sourcepath, "w");
    if (cgenctx->csource == NULL) {
        kbelog("couldn't open file output '%s'", cgenctx->sourcepath);
        exit(1);
    }
    cgenctx->valcount = 1;
    cgenctx->indent_level = 0;
    cgenctx->indent_max = 1;
    cgenctx->indent = kbmalloc(sizeof(cgenctx->indent[0]) * (cgenctx->indent_max + 1));
    cgenctx->indent[0] = '\0';
    kbstrpool_new(&cgenctx->vals);
    kbvec_new(&cgenctx->tmpvals, sizeof(int));
}

void kbcgenctx_del(struct kbcgenctx* cgenctx) {
    kbstr_del(&cgenctx->h_code);
    kbstr_del(&cgenctx->src_code);
    kbstr_del(&cgenctx->main_code);
    fclose(cgenctx->cheader);
    fclose(cgenctx->csource);
    kbfree(cgenctx->headerpath);
    kbfree(cgenctx->sourcepath);
    kbstrpool_del(&cgenctx->vals);
    kbvec_del(&cgenctx->tmpvals);
    kbfree(cgenctx->indent);
}

int kbcgen(struct kbopts* opts, struct kbsrc* src, struct kbast* ast, struct kbastinfo* astinfo, struct kbstr* exe) {
    struct kberrvec errvec = kberrvec_make();
    
    struct kbcgenctx cgenctx;
    kbcgenctx_new(opts, src, &cgenctx);

    emit(cgenctx.cheader, "// This file was generated by Kobalt compiler v%s\n", KBVERSION);
    
    char* headername = kbmalloc(sizeof(char) * (strlen(src->basename) + 2 + 8 + 1));
    memset(headername, 0, sizeof(char) * (strlen(src->basename) + 2 + 8 + 1));
    {
        int cur = 0;
        for(int ii = 0; ii < (int)strlen(src->basename)-3; ++ii) {
            if (src->basename[ii] >= 'a' && src->basename[ii] <= 'z') {
                headername[cur++] = src->basename[ii] + 'A' - 'a';
            }
            else if ((src->basename[ii] >= 'A' && src->basename[ii] <= 'Z') || (src->basename[ii] >= '0' && src->basename[ii] <= '9')) {
                headername[cur++] = src->basename[ii];
            }
        }
        headername[cur++] = '_';
        headername[cur++] = '_';
        genuidmaj(headername + cur);
    }

    emit(cgenctx.cheader, "#ifndef __KBH__%s\n", headername);
    emit(cgenctx.cheader, "#define __KBH__%s\n", headername);
    emit(cgenctx.cheader, "#include <stdlib.h>\n");
    emit(cgenctx.cheader, "#include <stdio.h>\n");
    emit(cgenctx.cheader, "#include <stdbool.h>\n");
    emit(cgenctx.cheader, "#include <stdint.h>\n");
    emit(cgenctx.cheader, "#include <math.h>\n");
    kbfree(headername);
    
    emit(cgenctx.csource, "// This file was generated by Kobalt compiler v%s\n", KBVERSION);
    emit(cgenctx.csource, "#include \"%s\"\n", cgenctx.include);
    
    cgen_rec(ast, astinfo, &cgenctx, 0);

    emit(cgenctx.csource, cgenctx.src_code.data);
    emit(cgenctx.csource, "\nint main() {\n");
    if (cgenctx.main_code.data) {
        emit(cgenctx.csource, cgenctx.main_code.data);
        emit(cgenctx.csource, "\treturn 0;\n");
    }
    emit(cgenctx.csource, "}\n");
   
    emit(cgenctx.cheader, "#endif\n");
    
    fflush(cgenctx.cheader);
    fflush(cgenctx.csource);

    char* srcname = cgenctx.sourcepath;
    for(char* p = cgenctx.sourcepath; *p != '\0'; ++p) {
        if(isds(*p)) {
            srcname = p + sizeof(p[0]);
        }
    }
    char* tmp = kbmalloc(sizeof(srcname[0]) * (strlen(srcname) + 1));
    strcpy(tmp, srcname);
    srcname = tmp;

    kberrvec_del(&errvec);    
    kbcgenctx_del(&cgenctx);

    kbilog("executable will be produced at %s", exe->data);

    kbstr_new(exe);
    kbstr_catf(exe, opts->cachedir);
    kbstr_catf(exe, "/");
    kbstr_catf(exe, src->basename);
#if WINDOWS
    kbstr_catf(exe, "0");
    strcpy(&exe->data[exe->len - 2], "exe");
#else
    exe->data[exe->len - 3] = '\0';
#endif

    struct kbcmdcc cmdcc;
    kbcmdcc_new(&cmdcc);
    kbcmdcc_compile(opts, &cmdcc, srcname, exe);
    kbcmdcc_del(&cmdcc);
    

    
    if (opts->output != NULL) {
        rename(exe->data, opts->output);
    }
        
    // TODO: create a file copy function in fs.c
    // FILE* srcexe = fopen(exefull, "rb");
    // if (srcexe == NULL) {
    //     kbelog("could not open file '%s'", exefull);
    //     exit(1);
    // }
    // FILE* destexe = fopen(opts->output, "wb");
    // if (destexe == NULL) {
    //     kbelog("could not open file '%s'", opts->output);
    //     exit(1);
    // }
    // char ch;
    // while ((ch = fgetc(srcexe)) != EOF) {
    //     fputc(ch, destexe);
    // }
    // fflush(destexe);
    // fclose(srcexe);
    // fclose(destexe);
    // kbfree(exe);
    kbfree(srcname);
    return 1;
}
