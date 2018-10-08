#include "kobalt/parser.h"
#include <stdlib.h>

struct Tree * parse (struct Array * tokens) {
    struct ArrayIterator it = array_it_make(tokens);

    while(!array_it_end(it)) {
        struct Token * token = array_it_get(it);
        token_debug(token);

        it = array_it_next(it);
    }

    return NULL;
}
