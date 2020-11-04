#include "kobalt/options.h"
#include "kobalt/source.h"
#include "kobalt/memory.h"
#include "kobalt/log.h"
#include "kobalt/fs.h"
#include "kobalt/uid.h"
#include "kobalt/time.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#if WINDOWS
#include <direct.h>
#else
#include <unistd.h>
#endif

void kbopts_new(int argc, char* argv[], struct kbopts* opts) {
    opts->stage = CODEGEN;
    opts->optim = 0;
    opts->run = 0;
    opts->output = NULL;
    opts->numsrcs = 0;
    int srcs_capacity = 1;
    opts->srcs = kbmalloc(sizeof(opts->srcs[0]) * srcs_capacity);
    opts->verbosity = 1;
    kbvec_new(&opts->exe_argv, sizeof(char*));

    unsigned int ii = 1;
    bool endopts = false;
    while (ii<(unsigned int)argc) {
        unsigned int jj = ii;
        if (strcmp(argv[jj], "--") == 0) {
            endopts = true;
        }
        else if (argv[jj][0] == '-') {
            if (strlen(argv[jj]) == 2) {
                char optopt = argv[jj][1];
                char * optarg = NULL;
                if (jj+1<(unsigned int)argc && argv[jj+1][0] != '-') {
                    optarg = argv[jj+1];
                }
                switch (optopt) {
                    case 'L':
                        opts->stage = LEX;
                        break;
                    case 'T':
                        opts->stage = PARSE;
                        break;
                    case 'v':
                        printf("Kobalt Language Compiler v%s\n\n", KBVERSION);
                        exit(0);
                        break;
                    case 'V':
                        opts->verbosity = 2;
                        break;
                    case 'r':
                        opts->run = 1;
                        break;
                    case 'h':
                        printf("Usage: %s [file...]\n", argv[0]);
                        printf("Options:\n");
                        printf("  -o  output file\n");
                        printf("  -V  enable verbose mode\n");
                        printf("  -v  display version\n");
                        printf("  -L  lexing stage\n");
                        printf("  -T  parsing stage\n");
                        exit(0);
                        break;
                    case 'o':
                        {
                            ii++;
                            if (optarg == NULL) {
                                kbelog("argument to '-o' is missing");
                                exit(1);
                            }
                            opts->output = (char*)kbmalloc(sizeof(opts->output[0]) * (strlen(optarg) + 1));
                            strcpy(opts->output, optarg);
                        }
                        break;
                    default: {
                        if (isprint (optopt)) {
                            kbelog("unknown option '-%c'", optopt);
                        }
                        else {
                            kbelog("unknown option character '\\x%x'", optopt);
                        }
                        exit(1);
                    }
                        break;
                }
            }
            else {
                kbelog("unknown option '%s'", argv[jj]);
                exit(1);
            }
        }
        else {
            if (endopts) {
                kbvec_push(&opts->exe_argv, &argv[jj]);
            }
            else {
                if (opts->numsrcs == srcs_capacity) {
                    srcs_capacity = 2 * srcs_capacity;
                    opts->srcs = kbrealloc(opts->srcs, sizeof(opts->srcs[0]) * srcs_capacity);
                }
                int st = 1;
                if (argv[jj][0] == '.') {
                    while(isds(argv[jj][st])) {
                        st ++;
                    }
                }
                // TODO: check if it is a file
                opts->srcs[opts->numsrcs] = &argv[jj][st];
                ++ opts->numsrcs;
            }
        }
        ii++;
    }

    kbvec_push(&opts->exe_argv, &(void*){NULL});

    if (!opts->numsrcs) {
        kbelog("no input file");
        exit(1);
    }

    size_t cwdsize = 32;
    opts->cwd = kbmalloc(cwdsize);
    int maxiter = 4;
    while(maxiter-- && (getcwd(opts->cwd, cwdsize) == NULL)) {
        cwdsize = 2 * cwdsize;
        opts->cwd = kbrealloc(opts->cwd, cwdsize);    
    } 
    if (maxiter == 0) {
        kbelog("allocation");
        exit(1);
    }
    
#if WINDOWS
    for(char* c = opts->cwd; *c != '\0'; c++) {
        if (*c == '\\') {
            *c = '/';
        }
    }
#endif
   
    seed(kbtime_get());

#if UNIX
    char* home = getenv("HOME");
    assert(home != NULL);

    int cachedirsize = strlen(home) + strlen("/.cache/kobalt") + 1 + 8 + 1;
    opts->cachedir = kbmalloc(cachedirsize);
    
    strcpy(opts->cachedir, home);
    strcat(opts->cachedir, "/.cache");
    ensuredir(opts->cachedir);

    strcat(opts->cachedir, "/kobalt/");
#else
    int cachedirsize = strlen("kbcache") + 1 + 8 + 1;
    opts->cachedir = kbmalloc(sizeof(char) * cachedirsize);
    strcpy(opts->cachedir, "kbcache/");
#endif
    ensuredir(opts->cachedir);
    opts->cachedir[cachedirsize - 1] = '\0';
    genuid(opts->cachedir + strlen(opts->cachedir));
    ensuredir(opts->cachedir);
}

char * kbstage_string(enum kbstage stage) {
    switch (stage) {
        case LEX:
            return "LEX";
        case PARSE:
            return "PARSE";
        case CODEGEN:
            return "CODEGEN";
    }
    return "UNDEFINED";
}

void kbopts_del(struct kbopts* opts) {
    kbfree(opts->cwd);
    kbfree(opts->srcs);
    kbfree(opts->cachedir);
    if (opts->output != NULL) {
        kbfree(opts->output);
    }
    kbfree(*(char**)kbvec_get(&opts->exe_argv, 0));
    kbvec_del(&opts->exe_argv);
}
