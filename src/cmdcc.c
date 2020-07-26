#include "kobalt/cmdcc.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

char* ccs[CCNone] = {"gcc", "clang"};
char* ccoptoutput[CCNone] = {"-o", "-o"};
char* ccoptextra[CCNone] = {"-std=c99", "-std=c99"};

void kbcmdcc_new(struct kbcmdcc* cmdcc) {
#if WINDOWS
    // TODO
    todo();    
#else
    for (int icc = 0; icc < CCNone; ++ icc) {
        int pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(1);
        }
        else if (pid == 0) {
            int devnull = open("/dev/null", O_WRONLY);
            if (devnull == -1) {
                perror("open");
                exit(1);
            }
            close(STDOUT_FILENO);
            dup(devnull);
            close(STDERR_FILENO);
            dup(devnull);
            close(devnull);
            execlp(ccs[icc], ccs[icc], "--version", NULL);
        }
        else {
            int status;
            wait(&status);
            if (WEXITSTATUS(status) == 0) {
                printf("info: found C compiler %d\n", icc);
                cmdcc->cc = icc;
                return;
            }
        }
    }

    fprintf(stderr, "error: could not find C compiler\n");
    exit(1);
#endif
}

void kbcmdcc_compile(struct kbcmdcc* cmdcc, char* src, char* bin) {
    int pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) {
        int cclog = open("kbcache/cc.log", O_RDWR | O_CREAT, S_IRWXU);
        if (cclog == -1) {
            perror("error: open");
            exit(1);
        }
        close(STDOUT_FILENO);
        dup(cclog);
        close(STDERR_FILENO);
        dup(cclog);
        close(cclog);
        execlp(ccs[cmdcc->cc], ccs[cmdcc->cc], ccoptextra[cmdcc->cc], ccoptoutput[cmdcc->cc], bin, src, NULL);
    }
    else {
        int status;
        wait(&status);
        if (WEXITSTATUS(status) == 0) {
            printf("info: Succesfully compiled %s\n", bin);
            return;
        }
        else {
            printf("error: Unexpected error when compiling C code. Output logs saved in kbcache/cc.log\n");
            exit(1);

        }
    }
}
