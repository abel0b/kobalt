#include "kobalt/fs.h"
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#if WINDOWS
#include <direct.h>
#else
#include <unistd.h>
#endif

void ensuredir(char* path) {
    struct stat st;
    if (stat(path, &st) == -1) {
#if WINDOWS
        if (mkdir(path) == -1) {
#else
        if (mkdir(path, 0700) == -1) {
#endif
            fprintf(stderr, "error: couldn't create directory '%s'\n", path);
            exit(1);
        }
    }
#if WINDOWS
    else if (!(st.st_mode & _S_IFDIR)) {
#else
    else if (S_ISREG(st.st_mode)) {
#endif
        fprintf(stderr, "kbc: error: couldn't create directory %s, path already exists\n", path);
        exit(1);
    }
}

int isds(char c) {
    return (c == '/') || (c == '\\');
}
