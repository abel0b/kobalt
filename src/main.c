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

int main(int argc, char * argv[]) {
    struct kbopts opts;
    kbopts_new(argc, argv, &opts);

    for(int ii=0; ii<opts.numsrcs; ++ii) {
        struct kbsrc src;
        kbsrc_new(opts.srcs[ii], &src);

        struct kbtoken* tokens;
        int numtokens;
        kblex(&src, &tokens, &numtokens);

        if (opts.stage == LEX) {
            for(int jj=0; jj<numtokens; jj++) kbtoken_display(&tokens[jj]);
            kbtoken_del_arr(numtokens, tokens);
            kbsrc_del(&src);
            continue;
        }
    
        struct kbast ast;
        kbparse(tokens, &src, &ast);

        if (opts.stage == PARSE) {
            kbast_display(&ast);
            kbtoken_del_arr(numtokens, tokens);
            kbast_del(&ast);
            kbsrc_del(&src);
            continue;
        }

        kbtypecheck(&ast);
        kbcgen(&opts, &src, &ast);

        kbtoken_del_arr(numtokens, tokens);
        kbast_del(&ast);
        kbsrc_del(&src);
    }

    kbopts_del(&opts);

    return EXIT_SUCCESS;
}
