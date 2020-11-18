#include <stdio.h>
#include <stdlib.h>
#include "kobalt/options.h"
#include "kobalt/ast.h"
#include "kobalt/lexer.h"
#include "kobalt/parser.h"
#include "kobalt/typecheck.h"
#include "kobalt/typeinfer.h"
#include "kobalt/cgen.h"
#include "kobalt/fs.h"
#include "kobalt/astinfo.h"
#include "kobalt/uid.h"
#include "kobalt/time.h"
#include "kobalt/log.h"
#include "kobalt/proc.h"
#include "kobalt/cmdcc.h"
#include "repl.h"
#include <stdlib.h>

#if UNIX
#if defined(__GLIBC__)
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

void handler(int sig) {
    void* array[10];
    size_t size;

    size = backtrace(array, 10);

    fprintf(stderr, "error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}
#endif
#endif

int main(int argc, char * argv[]) {
#if UNIX
#if defined(__GLIBC__)
    signal(SIGABRT, handler);
#endif
#endif

    if (argc == 1) {
        return kb_repl();
    }
    int status = EXIT_SUCCESS;
    struct kbopts opts;
    kbopts_new(argc, argv, &opts);
    struct kbtimer timer;

    for(int ii=0; ii<opts.numsrcs; ++ii) {
        struct kbsrc compiland;
        kbsrc_new(opts.srcs[ii], &compiland);

        if (opts.stage & LexingStage) {
            struct kbvec_token tokens;
            kbtimer_start(&timer);
            kblex(&compiland, &tokens);
            kbilog("step lex done in %dus", kbtimer_end(&timer));
            
            if (opts.stage >> 1 == 0) {
                FILE* out = (opts.output == NULL)? stdout: fopen(opts.output, "w");
                if (out == NULL) {
                    kbelog("could not open output file '%s'", opts.output);
                    exit(1);
                }
                for(int j = 0; j < tokens.size; ++ j) {
                    kbtoken_display(out, &tokens.data[j]);
                }
                if (opts.output != NULL) {
                    fclose(out);
                }
            }
            else {
                struct kbast ast;
                kbtimer_start(&timer);
                kbparse(&tokens, &compiland, &ast);
                kbilog("step parse done in %dus", kbtimer_end(&timer));
            
                if (opts.stage >> 2 == 0) {
                    FILE* out = (opts.output == NULL)? stdout: fopen(opts.output, "w");
                    if (out == NULL) {
                        kbelog("could not open output file '%s'", opts.output);
                        exit(1);
                    }
                    kbast_display(out, &ast, NULL);
                    if (opts.output != NULL) {
                        fclose(out);
                    }
                }
                else {
                    struct kbastinfo astinfo;
                    kbtimer_start(&timer);
                    kbtypeinfer(&ast, &astinfo);
                    kbilog("step typeinfer done in %dus", kbtimer_end(&timer));

                    kbtimer_start(&timer);
                    kbtypecheck(&ast, &astinfo);
                    kbilog("step typecheck done in %dus", kbtimer_end(&timer));

                    if (opts.stage >> 3 == 0) {
                        FILE* out = (opts.output == NULL)? stdout: fopen(opts.output, "w");
                        if (out == NULL) {
                            kbelog("could not open output file '%s'", opts.output);
                            exit(1);
                        }
                        kbast_display(out, &ast, &astinfo);
                        if (opts.output != NULL) {
                            fclose(out);
                        }
                    }
                    else {
                        struct kbstr exe, csrc;
                        kbtimer_start(&timer);
                        kbcgen(&opts, &compiland, &ast, &astinfo, &exe, &csrc);
                        kbilog("step cgen done in %dus", kbtimer_end(&timer));

                        if (opts.stage >> 4 == 0) {
                            FILE* out = (opts.output == NULL)? stdout: fopen(opts.output, "w");
                            if (out == NULL) {
                                kbelog("could not open output file '%s'", opts.output);
                                exit(1);
                            }
                            kbast_display(out, &ast, &astinfo);
                            if (opts.output != NULL) {
                                fclose(out);
                            }
                        }
                        else {
                            kbtimer_start(&timer);
                            struct kbcmdcc cmdcc;
                            kbcmdcc_new(&cmdcc);
                            kbcmdcc_compile(&opts, &cmdcc, &csrc, &exe);
                            kbcmdcc_del(&cmdcc);
                            kbilog("step cc done in %dus", kbtimer_end(&timer));
                                
                            if (opts.output != NULL) {
                                // TODO: copy file instead of renaming
                                rename(exe.data, opts.output);
                            }

                            if (opts.stage >> 5 == 0) {

                            }
                            else {
                                kbtimer_start(&timer);
                                status = kbspawn(exe.data, (char**)opts.exe_argv.data, NULL);
                                kbilog("process returned %d in %dus", status, kbtimer_end(&timer));
                            }
                        }
                            
                        kbstr_del(&exe);
                        kbstr_del(&csrc);
                    }
                    kbastinfo_del(&astinfo);
                }
                kbast_del(&ast);
            }

            for(int j = 0; j < tokens.size; ++ j) {
                kbtoken_del(&tokens.data[j]);
            }
            kbvec_token_del(&tokens);
        }
        kbsrc_del(&compiland);
    }

    kbopts_del(&opts);

    return status;
}
