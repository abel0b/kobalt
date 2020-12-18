#include "klbase/fs.h"
#include "klbase/log.h"
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
            kl_elog("couldn't create directory '%s'", path);
            exit(1);
        }
    }
#if WINDOWS
    else if (!(st.st_mode & _S_IFDIR)) {
#else
    else if (S_ISREG(st.st_mode)) {
#endif
        kl_elog("couldn't create directory %s, path already exists", path);
        exit(1);
    }
}

int isds(char c) {
#if WINDOWS
    return (c == '/') || (c == '\\');
#else
    return c == '/';
#endif
}

void read_to_str(char* path, struct kl_str* dest) {
    FILE * file = fopen(path, "rb");
    if (file == NULL) {
        perror("fopen");
        kl_elog("could not open file '%s'", path);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long int filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    kl_str_new(dest);

    if (filesize) {
        kl_str_resize(dest, filesize);
        size_t r = fread(dest->data, filesize, 1, file);
        assert(r == 1);
    }
   
    fclose(file);
}
