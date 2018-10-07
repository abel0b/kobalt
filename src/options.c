#include "array/array.h"
#include "kobalt/options.h"
#include "kobalt/source_file.h"
#include <stdio.h>
#define __USE_GNU
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


struct KobaltOptions *
kobalt_options_make(int argc, char * argv[]) {
    struct KobaltOptions * options = malloc (sizeof (struct KobaltOptions));
    options->cwd = get_current_dir_name ();
    options->source_files = array_make ();
    parse_argv(options, argc, argv);
    return options;
}

void kobalt_options_destroy(struct KobaltOptions * options) {
    free (options->cwd);
    array_destroy (options->source_files);
    free (options);
}

void parse_argv(struct KobaltOptions * options, int argc, char * argv[]) {
    for(int i = 1; i<argc; i++) {
        array_push(options->source_files, source_file_read(options, argv[i]));
    }
}
