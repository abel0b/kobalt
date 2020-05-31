#include "kobalt/options.h"
#include "kobalt/source.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#if WINDOWS
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

struct kbopts kbopts_make(int argc, char * argv[]) {
    struct kbopts options;
    options.stage = PARSE;
    options.output = stdout;
    options.num_srcs = 0;
    unsigned int srcs_capacity = 1;
    options.srcs = malloc(sizeof(struct kbsrc) * srcs_capacity);
    options.verbosity = 1;
    unsigned int ii = 1;
    while (ii<(unsigned int)argc) {
        unsigned int jj = ii;
        if (argv[jj][0] == '-') {
            if (strlen(argv[jj]) == 2) {
                char optopt = argv[jj][1];
                char * optarg = NULL;
                if (jj+1<(unsigned int)argc && argv[jj+1][0] != '-') {
                    optarg = argv[jj+1];
                }
                switch (optopt) {
                    case 'L':
                        options.stage = LEX;
                        break;
                    case 'T':
                        options.stage = PARSE;
                        break;
                    case 'V':
                        options.verbosity = 2;
                        break;
                    case 'h':
                        printf("Kobalt Language Compiler v%s\n\n", KBVERSION);
                        break;
                    case 'o': {
                        ii++;
                        if (optarg == NULL) {
                            fprintf(stderr, "Argument to '-o' is missing\n");
                            exit(1);
                        }
                        options.output = fopen(optarg, "w");
                        if (options.output == NULL) {
                            perror("Error opening output file");
                            exit(1);
                        }
                    }
                        break;
                    default: {
                        if (isprint (optopt)) {
                            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                        }
                        else {
                            fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                        }
                        exit(1);
                    }
                        break;
                }
            }
            else {
                fprintf (stderr, "Unknown option `%s`\n", argv[jj]);
                exit(1);
            }
        }
        else {
            if (options.num_srcs == srcs_capacity) {
                srcs_capacity = 2 * srcs_capacity;
                assert(realloc(options.srcs, sizeof(struct kbsrc) * srcs_capacity) != NULL);
            }
            options.srcs[options.num_srcs] = kbsrc_make(&options, argv[jj]);
            options.num_srcs ++;
        }
        ii++;
    }
#if DEBUG
    if(!options.num_srcs) {
        options.srcs[options.num_srcs] = kbsrc_make(&options, "examples/hello_world.kb");
        options.num_srcs++;
    }
#endif
    if (!options.num_srcs) {
        printf("error: no input file\n");
        exit(1);
    }

    size_t cwdsize = 32;
    options.cwd = malloc(cwdsize);
    int maxiter = 14;
    while(maxiter-- && (getcwd(options.cwd, cwdsize) == NULL)) {
        cwdsize = 2*cwdsize;
        options.cwd = realloc(options.cwd, cwdsize);    
    } 
    if (maxiter == 0) {
        fprintf(stderr, "Allocation error");
        exit(1);
    }
    return options;
}

char * kbstage_string(enum kbstage stage) {
    switch (stage) {
        case LEX:
            return "LEX";
        case PARSE:
            return "PARSE";
    }
    return "UNDEFINED";
}

void kbopts_display(struct kbopts * options) {
    fprintf(stderr, "kbopts {\n");
    fprintf(stderr, "  stage = %s\n", kbstage_string(options->stage));
    fprintf(stderr, "  cwd = %s\n", options->cwd);
    fprintf(stderr, "  num_srcs = %d\n", options->num_srcs);
    fprintf(stderr, "  srcs[%d] = [array]\n", options->num_srcs);
    fprintf(stderr, "}\n");
}

void kbopts_destroy(struct kbopts * options) {
    free (options->cwd);
    for(unsigned int i=0; i<options->num_srcs; i++) {
        kbsrc_destroy(&options->srcs[i]);
    }
    free(options->srcs);
}
