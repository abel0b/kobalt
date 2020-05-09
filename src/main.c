#include <stdio.h>
#include <stdlib.h>
#include "kobalt/options.h"
#include "kobalt/lexer.h"
#include "kobalt/parser.h"

int main(int argc, char * argv[]) {
    if (argc == 1) {
        printf("Kobalt Language Compiler v0.1.0\n\n");
        printf("error: no input file\n");
        exit(1);
    }

    struct kbopts options = kbopts_make(argc, argv);
    for(int ii=0; ii<options.num_srcs; ii++) {
        struct kblexer lexer = kblexer_make();
        unsigned int num_tokens = 0;
        struct kbtoken * tokens = kblexer_start(&lexer, &options.srcs[ii], &num_tokens);
        kblexer_destroy(&lexer);
        for(unsigned int jj=0; jj<num_tokens; jj++) {
            kbtoken_debug(&tokens[jj]);
        }
        free(tokens);
    }
    kbopts_destroy(&options);

    return EXIT_SUCCESS;
}
