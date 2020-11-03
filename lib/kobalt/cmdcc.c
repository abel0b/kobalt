#include "kobalt/cmdcc.h"
#include "kobalt/memory.h"
#include "kobalt/proc.h"
#include "kobalt/log.h"
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
char* ccs[CCNone] = {"clang", "gcc", "tcc"};
char* ccoptoutput[CCNone] = {"-o", "-o", "-o"};
char* ccoptextra[CCNone] = {"-std=c99", "-std=c99", ""};
#endif

void kbcmdcc_new(struct kbcmdcc* cmdcc) {
    for (int icc = 0; icc < CCNone; ++ icc) {
#if WINDOWS
        char command[64];
        if (snprintf(command, 32, "%s /help", ccs[icc]) >= 32) {
            kbelog("unexpected error");
            exit(1);
        }
        FILE* vpipe = _popen(command, "r");
        if (vpipe == NULL) {
            kbelog("spawn failed");
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
            if (dup(devnull) == -1) {
                perror("dup");
                exit(1);
            }
            close(STDERR_FILENO);
            if (dup(devnull) == -1) {
                perror("dup");
                exit(1);
            }
            close(devnull);
            execlp(ccs[icc], ccs[icc], "--version", NULL);
        }
        int status;
	    wait(&status);
        int exitstatus = WEXITSTATUS(status);
#endif
	    if (exitstatus == 0) {
	        kbilog("found C compiler '%s'", ccs[icc]);
            cmdcc->cc = icc;
            return;
        }
    }

    kbelog("could not find C compiler");
    exit(1);
}

void kbcmdcc_compile(struct kbopts* opts, struct kbcmdcc* cmdcc, char* src, struct kbstr* bin) {
    if (chdir(opts->cachedir) == -1) {
        perror("chdir");
        exit(1);
    };
    FILE* cclog = fopen("cc.log", "w");
    if (cclog == NULL) {
        kbelog("could not open file '%s/cc.log'", opts->cachedir);
        exit(1);
    }
    
#if WINDOWS
    // TODO: remove static path size limit
    char binopt[1024];
    int ret = snprintf(binopt, 1024, "%s%s", ccoptoutput[cmdcc->cc], bin->data);
    if (ret < 0 || ret > 1024) {
        kbelog("unexpected error, probably because of too long path");
        exit(1);
    }
    char* args[] = {ccoptextra[cmdcc->cc], binopt, src, NULL};
#else
    char* args[] = {ccoptextra[cmdcc->cc], ccoptoutput[cmdcc->cc], bin->data, src, NULL};
#endif

    int exitstatus = kbspawn(ccs[cmdcc->cc], args, cclog);
    if (exitstatus == 0) {
        kbilog("succesfully compiled %s", bin->data);
    }
    else {
        kbelog("C compilation exited with %d status. See logs saved in '%s/cc.log'", exitstatus, opts->cachedir);
        exit(1);
    }
    fclose(cclog);
    if (chdir(opts->cwd) == -1) {
        perror("chdir");
        exit(1);
    };
}

void kbcmdcc_del(struct kbcmdcc* cmdcc) {
    (void)cmdcc;
}
