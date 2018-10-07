#include <stdio.h>
#include "kobalt/options.h"
#include "array/array.h"
#include "kobalt/lexer.h"
#include "kobalt/parser.h"

int main(int argc, char * argv[]) {
    // printf("Kobalt Language Compiler v0.1.0\n\n");

    struct KobaltOptions * options = kobalt_options_make(argc, argv);

    struct ArrayIterator it = array_it_make(options->source_files);
    while(!array_it_end(it)) {
        struct SourceFile * source = array_it_get (it);
        struct Array * tokens = lex (source);
        struct Tree * ast = parse (tokens);

        it = array_it_next (it);
    }


    return 0;
}
