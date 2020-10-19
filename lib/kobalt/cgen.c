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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#define TAB "    "

struct kbfunctx {
    int hasreturn;
};

void kbfunctx_new(void* annot) {
    struct kbfunctx* functx = (struct kbfunctx*) annot;
    functx->hasreturn = 0;
}

void kbfunctx_del(void* functx) {
    unused(functx);
}

struct kbcgenctx {
    struct kbsrc* src;
    char* headerpath;
    FILE* cheader;
    char* sourcepath;
    char* include;
    FILE* csource;
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

static int cgen_rec(struct kbast* ast, struct kbcgenctx* ctx, int nid) {
    struct kbnode* node = kbast_getnode(ast, nid);

    kbilog("cgen %s", kbnode_kind_str(node->kind));

    switch(node->kind) {
        case NFile:
            for(int i = 0; i < node->data.group.numitems; ++i) {
                cgen_rec(ast, ctx, node->data.group.items[i]);
            }
            if (ctx->vals.objpool.num_elems > 0) {
                kbelog("unexpected state");
                exit(1);
            }
            break;
        case NCallParams:
            for(int i = 0; i < node->data.group.numitems; ++i) {
                cgen_rec(ast, ctx, node->data.group.items[i]);
            }
            break;
        case NSeq:
            {
                for(int i = 0; i < node->data.group.numitems - 1; ++i) {
                    cgen_rec(ast, ctx, node->data.group.items[i]);
                    kbstrpool_pop(&ctx->vals);
                    // TODO: display warning when expresion result is unused
                }
                cgen_rec(ast, ctx, node->data.group.items[node->data.group.numitems - 1]);
            }
            break;
        case NCallParam:
            cgen_rec(ast, ctx, node->data.callparam.expr);
            break;
        case NFun:
            {
                struct kbdict* scope = kbscope_get(ast, nid);
                char* name = kbast_getnode(ast, node->data.fun.id)->data.id.name;
                struct kbtype* type = kbdict_get(scope, name);
                {
                    char* out_type_str;
                    switch(type->data.fun.out_type->kind) {
                        case Unit:
                            out_type_str = "void";
                            break;
                        case Int:
                            out_type_str = "int";
                            break;
                        default:
                            todo(); // TODO: handle other type cases
                            break;
                    }
                    emit(ctx->csource, "\n%s%s %s(", ctx->indent, out_type_str, name);
                }

                int numparams = kbast_getnode(ast, node->data.fun.funparams)->data.group.numitems;
                int firstparam = 1;
                for(int i = 0; i < numparams; ++i) {
                    struct kbtype* in_type = * (struct kbtype**) kbvec_get(&type->data.fun.in_types, i);
                    if (in_type->kind != Unit) {
                        char* paramname = kbast_getnode(ast, kbast_getnode(ast, kbast_getnode(ast, node->data.fun.funparams)->data.group.items[i])->data.funparam.id)->data.id.name;
                        
                        char* in_type_str;
                        switch(in_type->kind) {
                            case Unit:
                                in_type_str = "void";
                                break;
                            case Int:
                                in_type_str = "int";
                                break;
                            default:
                                printf("%d\n", in_type->kind);
                                todo(); // TODO: handle other type cases
                                break;
                        }
                        if (!firstparam) {
                                emit(ctx->csource, ", ");
                                firstparam = 0;
                        }
                        emit(ctx->csource, "%s %s", in_type_str, paramname);
                    }
                    else if (numparams == 1) {
                        emit(ctx->csource, "void");
                    }
                }

                emit(ctx->csource, ")\n{\n");
                
                indent(ctx);
                cgen_rec(ast, ctx, node->data.fun.funbody);

                if (type->data.fun.out_type->kind != Unit) {
                    char* val = kbstrpool_pop(&ctx->vals);
                    emit(ctx->csource, "%sreturn %s;\n", ctx->indent, val);
                }
                dedent(ctx);
                emit(ctx->csource, "}\n");
            }
            break;
        case NStrLit:
            {
                kbstrpool_push(&ctx->vals, "\"%s\"", node->data.strlit.value);
            }
            break;
        case NIntLit:
            {
                kbstrpool_push(&ctx->vals, "%s", node->data.intlit.value);
            }
            break;
        case NFloatLit:
            {
                kbstrpool_push(&ctx->vals, "%s", node->data.floatlit.value);
            }
            break;
        case NCharLit:
            {
                kbstrpool_push(&ctx->vals, "%s", node->data.charlit.value);
            }
            break;
        case NId:
            {
                kbstrpool_push(&ctx->vals, "%s", node->data.id.name);
            }
            break;
        case NCall:
            {
                cgen_rec(ast, ctx, node->data.call.callparams);
                struct kbnode* sym = kbast_getnode(ast, node->data.call.id);
                if (sym->kind == NId) {
                    char* kbname = sym->data.id.name;
                    char* cname;
                    if (strcmp(kbname, "print") == 0) {
                        cname = "printf";
                    }
                    else {
                        kbelog("undefined function %s", kbname);
                        // exit(1);
                        cname = kbname;
                    }

                    int retval = ctx->valcount ++;
                    
                    emit(ctx->csource, "%s%s val%d = ", ctx->indent, "int", retval);
                    if (strcmp(kbname, "print") == 0) {
                        emit(ctx->csource, "%s(\"%%s\", ", cname);
                    }
                    else {
                        emit(ctx->csource, "%s(", cname);
                    }

                    char* val = kbstrpool_pop(&ctx->vals);
                    emit(ctx->csource, "%s", val);
                    emit(ctx->csource, ");\n");
                    
                    kbstrpool_push(&ctx->vals, "val%d", retval);
                }
                else {
                    assert(sym->kind == NSym);
                    char* op = specials[sym->data.sym.kind];
                    int retval = ctx->valcount++;
                    char* val2 = kbstrpool_pop(&ctx->vals);
                    char* val1 = kbstrpool_pop(&ctx->vals);
                    emit(ctx->csource, "%s%s val%d = %s %s %s;\n", ctx->indent, "int", retval, val1, op, val2);
                    kbstrpool_push(&ctx->vals, "val%d", retval);
                }
            }
            break;
        case NIfElse:
            {
                int val = ctx->valcount++;
                emit(ctx->csource, "%s%s val%d;\n", ctx->indent, "int", val);
                kbvec_push(&ctx->tmpvals, &val);
                for(int i = 0; i < node->data.group.numitems; ++i) {
                    cgen_rec(ast, ctx, node->data.group.items[i]);
                }
                kbvec_pop(&ctx->tmpvals, NULL);
                kbstrpool_push(&ctx->vals, "val%d");
            }
            break;
        case NIfBranch:
        case NElifBranch:
            {
                int retval;
                kbvec_peek(&ctx->tmpvals, &retval);
                cgen_rec(ast, ctx, node->data.ifbranch.cond);
                char* condval = kbstrpool_pop(&ctx->vals);
                if (node->kind == NElifBranch) {
                    emit(ctx->csource, "%selse\n%s{\n", ctx->indent, ctx->indent);
                    indent(ctx);
                }
                emit(ctx->csource, "%sif (%s)\n\t{\n", ctx->indent, condval);
                indent(ctx);

                cgen_rec(ast, ctx, node->data.ifbranch.conseq);
                char* branchval = kbstrpool_pop(&ctx->vals);
                emit(ctx->csource, "%sval%d = %s;\n", ctx->indent, retval, branchval);
                dedent(ctx);
                emit(ctx->csource, "%s}\n", ctx->indent);
                if (node->kind == NElifBranch) {
                    dedent(ctx);
                    emit(ctx->csource, "%s}\n", ctx->indent);
                }
            }
            break;
        case NElseBranch:
            {
                int retval;
                kbvec_peek(&ctx->tmpvals, &retval);
                emit(ctx->csource, "%selse\n%s{\n", ctx->indent, ctx->indent);
                indent(ctx);
                cgen_rec(ast, ctx, node->data.ifbranch.conseq);
                char* branchval = kbstrpool_pop(&ctx->vals);
                emit(ctx->csource, "%sval%d = %s;\n", ctx->indent, retval, branchval);
                dedent(ctx);
                emit(ctx->csource, "%s}\n", ctx->indent);
            }
            break;
        default:
            break;
    }
    return 1;
}

void kbcgenctx_new(struct kbopts* opts, struct kbsrc* src, struct kbcgenctx* cgenctx) {
    cgenctx->src = src;
    int lencwd = strlen(opts->cwd);
    int lencachedir = strlen(opts->cachedir);
    int srcsize = lencwd + 1 + lencachedir + 1 + strlen(src->filename);
    
    cgenctx->headerpath = kbmalloc(sizeof(char) * srcsize);
    cgenctx->sourcepath = kbmalloc(sizeof(char) * srcsize);
    
    sprintf(cgenctx->headerpath, "%s/%s/", opts->cwd, opts->cachedir);
    int i;
    cgenctx->headerpath[lencwd] = '/';
    cgenctx->include = &cgenctx->headerpath[lencwd + 1 + lencachedir + 1];
    for(i = 0; i<(int)strlen(src->filename)-3; i++) {
        cgenctx->headerpath[lencwd + 1 + lencachedir + 1 + i] = (isds(src->filename[i]))? '%' : src->filename[i];
    }
    cgenctx->headerpath[lencwd + 1 + lencachedir + 1 + i] = '.';
    cgenctx->headerpath[lencwd + 1 + lencachedir + 1 + i + 1] = 'h';
    cgenctx->headerpath[lencwd + 1 + lencachedir + 1 + i + 2] = '\0';
    cgenctx->cheader = fopen(cgenctx->headerpath, "w");
    if (cgenctx->cheader == NULL) {
        kbelog("couldn't open output file '%s'", cgenctx->headerpath);
        exit(1);
    }
   
    strcpy(cgenctx->sourcepath, cgenctx->headerpath);
    cgenctx->sourcepath[lencwd + 1 + lencachedir + 1 + i + 1] = 'c';
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
    fclose(cgenctx->cheader);
    fclose(cgenctx->csource);
    kbfree(cgenctx->headerpath);
    kbfree(cgenctx->sourcepath);
    kbstrpool_del(&cgenctx->vals);
    kbvec_del(&cgenctx->tmpvals);
    kbfree(cgenctx->indent);
}

int kbcgen(struct kbopts* opts, struct kbsrc* src, struct kbast* ast) {
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
    
    cgen_rec(ast, &cgenctx, 0);
   
    emit(cgenctx.cheader, "#endif\n");
    
    fflush(cgenctx.cheader);
    fflush(cgenctx.csource);
    
    char* exename = cgenctx.src->basename;
    for(char* c = cgenctx.src->basename; *c != '\0'; ++c) {
        if (isds(*c)) {
            exename = c + sizeof(c[0]);
        }
    }

    char* exe = kbmalloc(sizeof(exename) * (strlen(exename) + 4));
    strcpy(exe, exename);
    int lenexename = strlen(exename);
    for(int i = 0; i < lenexename; ++ i) {
        if (exe[lenexename - 1 - i] == '.') {
#if WINDOWS
            strcpy(&exe[lenexename - i], "exe");
#else
            exe[lenexename - 1 - i] = '\0';
#endif
            break;
        }
    }

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

    struct kbcmdcc cmdcc;
    kbcmdcc_new(&cmdcc);
    kbcmdcc_compile(opts, &cmdcc, srcname, exe);
    kbcmdcc_del(&cmdcc);
    
    if (opts->output != NULL) {
        char* exefull = kbmalloc(sizeof(exefull[0]) * (strlen(opts->cachedir) + 1 + strlen(exe) + 1));
        strcpy(exefull, opts->cachedir);
        exefull[strlen(opts->cachedir)] = '/';
        strcpy(&exefull[strlen(opts->cachedir) + 1], exe);
        rename(exefull, opts->output);
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
        kbfree(exefull);
    }

    kbfree(exe);
    kbfree(srcname);
    return 1;
}
