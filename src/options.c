#include "kobalt/options.h"
#include "kobalt/source.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct kbopts kbopts_make(int argc, char * argv[]) {
    struct kbopts options;
    size_t cwdsize = 32;
    options.cwd = malloc(cwdsize);
    while(getcwd(options.cwd, cwdsize) == NULL) {
        cwdsize = 2*cwdsize;
        options.cwd = realloc(options.cwd, cwdsize);    
    } 
    options.num_srcs = argc-1;
    options.srcs = malloc(sizeof(struct kbsrc) * options.num_srcs);
    for(int i=0; i<options.num_srcs; i++) {
        options.srcs[i] = kbsrc_make(&options, argv[1+i]);
    }
    return options;
}

void kbopts_destroy(struct kbopts * options) {
    free (options->cwd);
    for(int i=0; i<options->num_srcs; i++) {
        kbsrc_destroy(&options->srcs[i]);
    }
}
