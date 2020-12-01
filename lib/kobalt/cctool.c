#include "kobalt/cctool.h"
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
char* ccoptout[CCNone] = {"/Fe", "/Fe"};
char* ccoptobj[CCNone] = {"", ""};
char* ccoptobjout[CCNone] = {"/Fo", "/Fo"};
char* ccopt1[CCNone] = {"", ""};
char* ccopt2[CCNone] = {"", ""};
#else
char* ccs[CCNone] = {"clang", "gcc", "tcc"};
char* ccoptobj[CCNone] = {"-c", "-c", "-c"};
char* ccoptout[CCNone] = {"-o", "-o", "-o"};
char* ccoptobjout[CCNone] = {"-o", "-o", "-o"};
char* ccopt1[CCNone] = {"-std=c99", "-std=c99", ""};
char* ccopt2[CCNone] = {"-pedantic", "-pedantic", ""};
#endif

void kbcmdcc_new(struct kbcmdcc* cmdcc) {
    for (int icc = 0; icc < CCNone; ++ icc) {
#if WINDOWS
        // TODO: use CreateProcess
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
	        // kbilog("found C compiler '%s'", ccs[icc]);
            cmdcc->cc = icc;
            return;
        }
    }

    kbelog("could not find C compiler");
    exit(1);
}

int kbcmdcc_compile(struct kbopts* opts, struct kbcmdcc* cmdcc, struct kbstr* src) {
    struct kbstr obj;
    kbstr_new(&obj);
    kbstr_cat(&obj, src->data);
    obj.data[obj.len - 1] = 'o';

    struct kbstr logfilepath;
    kbstr_new(&logfilepath);
    kbstr_catf(&logfilepath, "%s/cc.log", opts->cachepath.data);
    FILE* cclog = fopen(logfilepath.data, "w+");
    if (cclog == NULL) {
        kbelog("could not open file '%s/cc.log'", opts->cachepath.data);
        exit(1);
    }
    kbstr_del(&logfilepath);

    struct kbvec_cstr args;
    kbvec_cstr_new(&args);
    kbvec_cstr_push(&args, ccopt1[cmdcc->cc]);
    kbvec_cstr_push(&args, ccopt2[cmdcc->cc]);
    kbvec_cstr_push(&args, ccoptobj[cmdcc->cc]);

#if WINDOWS
    // TODO: remove static path size limit
    char binopt[1024];
    int ret = snprintf(binopt, 1024, "%s%s", ccoptobjout[cmdcc->cc], obj.data);
    if (ret < 0 || ret > 1024) {
        kbelog("unexpected error, probably because of too long path");
        exit(1);
    }
    kbvec_cstr_push(&args, binopt);
#else
    kbvec_cstr_push(&args, ccoptobjout[cmdcc->cc]);
    kbvec_cstr_push(&args, obj.data);
#endif

    kbvec_cstr_push(&args, src->data);
    kbvec_cstr_push(&args, NULL);

    int exitstatus = kbspawn(ccs[cmdcc->cc], args.data, cclog);
    if (exitstatus == 0) {
        // kbilog("succesfully compiled %s", obj.data);
    }
    else {
        kbelog("C compilation exited with %d status. See logs saved in '%s/cc.log'", exitstatus, opts->cachepath.data);
#if DEBUG
        // struct kbstr logpath;
        // kbstr_new(&logpath);
        // kbstr_cat(&logpath, opts->cachepath.data);
        // kbpath_push(&logpath, "cc.log");
        // FILE* cclog = fopen(logpath.data, "r");
        // kbstr_del(&logpath),
        fseek(cclog, 0, SEEK_END);
        long int filesize = ftell(cclog);
        fseek(cclog, 0, SEEK_SET);
        
        char* content = kbmalloc(sizeof(char) * (filesize + 1));
        content[filesize] = '\0';
        fread(content, filesize, 1, cclog);
        fprintf(stderr, "%s\n", content);
        kbfree(content);
#endif
    }
    fclose(cclog);
    kbvec_cstr_del(&args);
    kbstr_del(&obj);
    return exitstatus;
}

int kbcmdcc_link(struct kbopts* opts, struct kbcmdcc* cmdcc, struct kbvec_str* objs, struct kbstr* bin) {
    struct kbstr logfilepath;
    kbstr_new(&logfilepath);
    kbstr_catf(&logfilepath, "%s/link.log", opts->cachepath.data);
    FILE* linklog = fopen(logfilepath.data, "w+");
    if (linklog == NULL) {
        kbelog("could not open file '%s/cc.log'", opts->cachepath.data);
        exit(1);
    }
    kbstr_del(&logfilepath);

    struct kbvec_cstr args;
    kbvec_cstr_new(&args);
#if WINDOWS
    // TODO: remove static path size limit
    char binopt[1024];
    int ret = snprintf(binopt, 1024, "%s%s", ccoptout[cmdcc->cc], bin->data);
    if (ret < 0 || ret > 1024) {
        kbelog("unexpected error, probably because of too long path");
        exit(1);
    }
    kbvec_cstr_push(&args, binopt);
#else
    kbvec_cstr_push(&args, ccoptout[cmdcc->cc]);
    kbvec_cstr_push(&args, bin->data);
#endif

    for (int i = 0; i < objs->size; ++ i) {
        kbvec_cstr_push(&args, objs->data[i].data);
    }
    kbvec_cstr_push(&args, NULL);

    int exitstatus = kbspawn(ccs[cmdcc->cc], args.data, linklog);
    if (exitstatus) {
        kbelog("Linking exited with %d status. See logs saved in '%s/link.log'", exitstatus, opts->cachepath.data);
#if DEBUG
        fseek(linklog, 0, SEEK_END);
        long int filesize = ftell(linklog);
        fseek(linklog, 0, SEEK_SET);
        
        char* content = kbmalloc(sizeof(char) * (filesize + 1));
        content[filesize] = '\0';
        fread(content, filesize, 1, linklog);
        fprintf(stderr, "%s\n", content);
        kbfree(content);
#endif
    }
    fclose(linklog);
    kbvec_cstr_del(&args);
    return exitstatus;
}

void kbcmdcc_del(struct kbcmdcc* cmdcc) {
    (void)cmdcc;
}

int kbcc(struct kbopts* opts, struct kbstr* src) {
    struct kbcmdcc cmdcc;
    kbcmdcc_new(&cmdcc);
    int status = kbcmdcc_compile(opts, &cmdcc, src);
    kbcmdcc_del(&cmdcc);
    return status;
}

int kblink(struct kbopts* opts, struct kbvec_str* objs, struct kbstr* bin) {
    struct kbcmdcc cmdcc;
    kbcmdcc_new(&cmdcc);
    int status = kbcmdcc_link(opts, &cmdcc, objs, bin);
    kbcmdcc_del(&cmdcc);
    return status;
}
