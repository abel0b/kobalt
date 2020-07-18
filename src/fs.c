#include "kobalt/fs.h"
#if WINDOWS

#else
#include <sys/stat.h>
#endif
#include <stdio.h>

void ensuredir(char* path) {
#if WINDOWS
    // TODO
    todo();
#else
    struct stat st;
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0700) == -1) {
            fprintf(stderr, "error: couldn't create directory '%s'\n", path);
        }
    }
    else if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "error: couldn't create directory %s, path already exists\n", path);
    }
#endif
}
