#include "kobalt/cmdcc.h"
#include "kobalt/memory.h"
#include "kobalt/proc.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#if WINDOWS
#include <process.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#if WINDOWS
char* ccs[CCNone] = {"cl", "clang-cl"};
char* ccoptoutput[CCNone] = {"/Fe", "/Fe"};
char* ccoptextra[CCNone] = {"", ""};
#else
char* ccs[CCNone] = {"gcc", "clang"};
char* ccoptoutput[CCNone] = {"-o", "-o"};
char* ccoptextra[CCNone] = {"-std=c99", "-std=c99"};
#endif

void kbcmdcc_new(struct kbcmdcc* cmdcc) {
    for (int icc = 0; icc < CCNone; ++ icc) {
#if WINDOWS
        char command[64];
        if (snprintf(command, 32, "%s /help", ccs[icc]) >= 32) {
            fprintf(stderr, "error: unexpected error\n");
            exit(1);
        }
        FILE* vpipe = _popen(command, "r");
        if (vpipe == NULL) {
            fprintf(stderr, "error: spawn failed\n");
            exit(1);
        }
        
        // size_t len;
        // char* buf;
        // fseek(vpipe, 0, SEEK_END);
        // len = ftell(vpipe);
        // fseek(vpipe, 0, SEEK_SET);
        // cmdcc->version = kbmalloc((len + 1) * sizeof(cmdcc->version[0]));
        // cmdcc->version[len] = '\0';
        // assert(fread(cmdcc->version, len, 1, vpipe  ) == (unsigned long)1); 
    
        int exitstatus = _pclose(vpipe);
#else
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
        int status;
	    wait(&status);
        int exitstatus = WEXITSTATUS(status);
#endif
	    if (exitstatus == 0) {
	        printf("info: found C compiler '%s'\n", ccs[icc]);
            cmdcc->cc = icc;
            return;
        }
    }

    fprintf(stderr, "error: could not find C compiler\n");
    exit(1);
}

void kbcmdcc_compile(struct kbopts* opts, struct kbcmdcc* cmdcc, char* src, char* bin) {
    chdir(opts->cachedir);
    FILE* cclog = fopen("cc.log", "w");
    if (cclog == NULL) {
        fprintf(stderr, "kbc: error: could not open file '%s/cc.log'\n", opts->cachedir);
        exit(1);
    }
    
#if WINDOWS
    // TODO: remove static path size limit
    char binopt[1024];
    int ret = snprintf(binopt, 1024, "%s%s", ccoptoutput[cmdcc->cc], bin);
    if (ret < 0 || ret > 1024) {
        fprintf(stderr, "error: unexpected error, probably because of too long path\n");
        exit(1);
    }
    char* argv[] = {ccs[cmdcc->cc], ccoptextra[cmdcc->cc], binopt, src, NULL};
#else
    char* argv[] = {ccs[cmdcc->cc], ccoptextra[cmdcc->cc], ccoptoutput[cmdcc->cc], bin, src, NULL};
#endif

    int exitstatus = kbspawn(argv, cclog);
    if (exitstatus == 0) {
        printf("info: succesfully compiled %s\n", bin);
    }
    else {
        printf("error: C compilation exited with %d status, logs saved in '%s/cc.log'", exitstatus, opts->cachedir);
        exit(1);
    }
    fclose(cclog);
    chdir(opts->cwd);
}

void kbcmdcc_del(struct kbcmdcc* cmdcc) {

}
