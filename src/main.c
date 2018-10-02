#include <stdio.h>
#include "kobalt/lexer.h"
#include "kobalt/options.h"

int main(int argc, char * argv[]) {
    printf("Kobalt Language Compiler v0.1.0\n\n");

    struct KobaltOptions * options = kobalt_options_make(argc, argv);
    // struct Array * tokens = lex (source_program)
    //struct tree * ast = parse(tokens)


    return 0;
}
