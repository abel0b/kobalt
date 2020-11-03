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
#include "repl.h"
#include <stdlib.h>

#if UNIX
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

int main(int argc, char * argv[]) {
#if UNIX
    signal(SIGABRT, handler);
#endif

    if (argc == 1) {
        return kb_repl();
    }
    struct kbopts opts;
    kbopts_new(argc, argv, &opts);
    struct kbtimer timer;

    for(int ii=0; ii<opts.numsrcs; ++ii) {
        struct kbsrc src;
        kbsrc_new(opts.srcs[ii], &src);

        struct kbtoken* tokens;
        int numtokens;
        kbtimer_start(&timer);
        kblex(&src, &tokens, &numtokens);
        kbilog("step lex done in %dus", kbtimer_end(&timer));

        if (opts.stage == LEX) {
            FILE* out = (opts.output == NULL)? stdout: fopen(opts.output, "w");
            if (out == NULL) {
                kbelog("could not open output file '%s'", opts.output);
                exit(1);
            }
            for(int jj=0; jj<numtokens; jj++) kbtoken_display(out, &tokens[jj]);
            if (opts.output != NULL) fclose(out);
            kbtoken_del_arr(numtokens, tokens);
            kbsrc_del(&src);
            continue;
        }
    
        struct kbast ast;
        kbtimer_start(&timer);
        kbparse(tokens, numtokens, &src, &ast);
        kbilog("step parse done in %dus", kbtimer_end(&timer));

        if (opts.stage == PARSE) {
            FILE* out = (opts.output == NULL)? stdout : fopen(opts.output, "w");
            if (out == NULL) {
                kbelog("could not open output file '%s'", opts.output);
                exit(1);
            }
            kbast_display(out, &ast);
            if (opts.output != NULL) fclose(out);
            kbtoken_del_arr(numtokens, tokens);
            kbast_del(&ast);
            kbsrc_del(&src);
            continue;
        }

        struct kbastinfo astinfo;
        kbtimer_start(&timer);
        kbtypeinfer(&ast, &astinfo);
        kbilog("step typeinfer done in %dus", kbtimer_end(&timer));

        kbtimer_start(&timer);
        kbtypecheck(&ast, &astinfo);
        kbilog("step typecheck done in %dus", kbtimer_end(&timer));

        struct kbstr exe;
        kbtimer_start(&timer);
        kbcgen(&opts, &src, &ast, &astinfo, &exe);
        kbilog("step cgen and compile done in %dus", kbtimer_end(&timer));

        kbtimer_start(&timer);
#ifdef DEBUG
        for(int i = 0; i < opts.exe_argv.num_elems - 1; ++i) {
            char* arg = *(char**)kbvec_get(&opts.exe_argv, i);
            kbilog("arg.%d = \"%s\"", i, arg);
        }
#endif

        kbtoken_del_arr(numtokens, tokens);
        kbastinfo_del(&astinfo);
        kbast_del(&ast);

        int status = kbspawn(exe.data, (char**)opts.exe_argv.elems, NULL);
        kbilog("step execute done in %dus", kbtimer_end(&timer));
        kbilog("process returned %d", status);

        kbsrc_del(&src);
        kbstr_del(&exe);
    }

    kbopts_del(&opts);

    return EXIT_SUCCESS;
}
