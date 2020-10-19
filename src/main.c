#include <stdio.h>
#include <stdlib.h>
#include "kobalt/options.h"
#include "kobalt/ast.h"
#include "kobalt/lexer.h"
#include "kobalt/parser.h"
#include "kobalt/typecheck.h" 
#include "kobalt/cgen.h"
#include "kobalt/fs.h"
#include "kobalt/uid.h"
#include "kobalt/time.h"
#include "kobalt/log.h"
#include "repl.h"

int main(int argc, char * argv[]) {
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

        kbtimer_start(&timer);
        kbtypecheck(&ast);
        kbilog("step typecheck done in %dus", kbtimer_end(&timer));

        kbtimer_start(&timer);
        kbcgen(&opts, &src, &ast);
        kbilog("step cgen and compile done in %dus", kbtimer_end(&timer));

        kbtoken_del_arr(numtokens, tokens);
        kbast_del(&ast);
        kbsrc_del(&src);
    }

    kbopts_del(&opts);

    return EXIT_SUCCESS;
}
