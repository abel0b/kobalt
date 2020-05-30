#include <stdio.h>
#include <stdlib.h>
#include "kobalt/options.h"
#include "kobalt/lexer.h"
#include "kobalt/parser.h"
#include "kobalt/ast.h"

int main(int argc, char * argv[]) {
    struct kbopts options = kbopts_make(argc, argv);
    for(unsigned int ii=0; ii<options.num_srcs; ii++) {
        // LEX
        struct kblexer lexer = kblexer_make();
        unsigned int num_tokens = 0;
        struct kbtoken * tokens = kblexer_start(&lexer, &options.srcs[ii], &num_tokens);
        kblexer_destroy(&lexer);
        if (options.stage == LEX) {
            for(unsigned int jj=0; jj<num_tokens; jj++) {
                kbtoken_display(&tokens[jj]);
            }
            kbtoken_destroy_arr(num_tokens, tokens);
            continue;
        }
    
        // PARSE
        struct kbparser parser = kbparser_make(tokens, &options.srcs[ii]);
        kbparse(&parser);
        kbast_display(parser.ast);
        kbparser_destroy(&parser);

        kbtoken_destroy_arr(num_tokens, tokens);
    }
    kbopts_destroy(&options);

    return EXIT_SUCCESS;
}
