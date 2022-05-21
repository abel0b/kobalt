#include "abl/fs.h"
#include "abl/log.h"
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#if WINDOWS
#include <direct.h>
#else
#include <unistd.h>
#endif

ABL_API void abl_fs_mkdirp(char* path) {
    struct stat st;
    if (stat(path, &st) == -1) {
#if WINDOWS
        if (mkdir(path) == -1) {
#else
        if (mkdir(path, 0700) == -1) {
#endif
            abl_elog("couldn't create directory '%s'", path);
            exit(1);
        }
    }
#if WINDOWS
    else if (!(st.st_mode & _S_IFDIR)) {
#else
    else if (S_ISREG(st.st_mode)) {
#endif
        abl_elog("couldn't create directory %s, path already exists", path);
        exit(1);
    }
}

ABL_API void abl_fs_read2str(char* path, struct abl_str* dest) {
    FILE * file = fopen(path, "rb");
    if (file == NULL) {
        perror("fopen");
        abl_elog("could not open file '%s'", path);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long int filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    abl_str_new(dest);

    if (filesize) {
        abl_str_resize(dest, filesize);
        fread(dest->data, filesize, 1, file);
    }
   
    fclose(file);
}
