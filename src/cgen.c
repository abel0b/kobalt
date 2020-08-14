#include "kobalt/cgen.h"
#include "kobalt/error.h"
#include "kobalt/options.h"
#include "kobalt/memory.h"
#include "kobalt/uid.h"
#include "kobalt/cmdcc.h"
#include "kobalt/fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define TAB "    "

struct kbcgenctx {
    struct kbsrc* src;
    char* headerpath;
    FILE* cheader;
    char* sourcepath;
    char* include;
    FILE* csource;
};

void emit(FILE* file, char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(file, fmt, args);
    va_end(args);
} 

static int cgen_aux(struct kbastvisitor * astvisitor) {
    struct kbnode* node = kbast_getnode(astvisitor->ast, astvisitor->cur.nid);
    struct kbcgenctx* cgenctx = astvisitor->ctx;

    switch(node->kind) {
        case NFile:
            break;
        case NFun:
            {
                char* name = kbast_getnode(astvisitor->ast, node->data.fun.id)->data.id.name;
                if (strcmp(name, "main") == 0) {
                    emit(cgenctx->csource, "int main() {\n");
                    emit(cgenctx->csource, TAB "printf(\"Hello world!\\n\");\n");
                    emit(cgenctx->csource, TAB "return 0;\n");
                    emit(cgenctx->csource, "}\n");
                }
            }
            break;
        case NFunParam:
            break;
        case NFunParams:
            break;
        case NFunBody:
            break;
        case NDecl:
            break;
        case NType:
            break;
        case NExpr:
            break;
        case NTerm:
            break;
        case NBinExpr:
            break;
        case NStrLit:
            break;
        case NIntLit:
            break;
        case NFloatLit:
            break;
        case NCharLit:
            break;
        case NCall:
            break;
        case NAssign:
            break;
        case NId:
            break;
        case NCallParams:
            break;
        case NCallParam:
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
        cgenctx->headerpath[lencwd + 1 + lencachedir + 1 + i] = (src->filename[i] != '/')? src->filename[i] : '%';
    }
    cgenctx->headerpath[lencwd + 1 + lencachedir + 1 + i] = '.';
    cgenctx->headerpath[lencwd + 1 + lencachedir + 1 + i + 1] = 'h';
    cgenctx->headerpath[lencwd + 1 + lencachedir + 1 + i + 2] = '\0';
    cgenctx->cheader = fopen(cgenctx->headerpath, "w");
    if (cgenctx->cheader == NULL) {
        fprintf(stderr, "kbc: error: couldn't open output file '%s'", cgenctx->headerpath);
        exit(1);
    }
   
    strcpy(cgenctx->sourcepath, cgenctx->headerpath);
    cgenctx->sourcepath[lencwd + 1 + lencachedir + 1 + i + 1] = 'c';
    cgenctx->csource = fopen(cgenctx->sourcepath, "w");
    if (cgenctx->csource == NULL) {
        fprintf(stderr, "kbc: error: couldn't open file output '%s'", cgenctx->sourcepath);
        exit(1);
    }
}

void kbcgenctx_del(struct kbcgenctx* cgenctx) {
    fclose(cgenctx->cheader);
    fclose(cgenctx->csource);
    kbfree(cgenctx->headerpath);
    kbfree(cgenctx->sourcepath);
}

int kbcgen(struct kbopts* opts, struct kbsrc* src, struct kbast* ast) {
    struct kberrvec errvec = kberrvec_make();
    
    struct kbcgenctx cgenctx;
    kbcgenctx_new(opts, src, &cgenctx);

    struct kbastvisitor astvisitor;
    kbastvisitor_new(ast, &cgenctx, cgen_aux, &astvisitor);
    

    emit(cgenctx.cheader, "// This file was generated by kbc v%s\n", KBVERSION);
    
    char* headername = kbmalloc(sizeof(char) * (strlen(src->basename) + 2 + 8 + 1));
    memset(headername, 0, sizeof(char) * (strlen(src->basename) + 2 + 8 + 1));
    {
        int cur = 0;
        for(int ii = 0; ii < (int)strlen(src->basename)-3; ++ii) {
            if (src->basename[ii] >= 'a' && src->basename[ii] <= 'z') {
                headername[cur++] = src->basename[ii] + 'A' - 'a';
            }
            else if ((src->basename[ii] >= 'A' && src->basename[ii] <= 'A') || (src->basename[ii] >= '0' && src->basename[ii] <= '9')) {
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
    
    emit(cgenctx.csource, "// This file was generated by kbc v%s\n", KBVERSION);
    emit(cgenctx.csource, "#include \"%s\"\n", cgenctx.include);
    emit(cgenctx.csource, "\n");
    
    kbastvisitor_run(&astvisitor);
   
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

    kbastvisitor_del(&astvisitor);
    kberrvec_del(&errvec);    
    kbcgenctx_del(&cgenctx);

    struct kbcmdcc cmdcc;
    kbcmdcc_new(&cmdcc);
    kbcmdcc_compile(opts, &cmdcc, srcname, exe);
    kbcmdcc_del(&cmdcc);
    kbfree(exe);
    kbfree(srcname);
    return 1;
}
