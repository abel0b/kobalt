#include "kobalt/cctool.h"
#include "abl/mem.h"
#include "abl/proc.h"
#include "abl/log.h"
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
char* ccoptobj[CCNone] = {"/c", "/c"};
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

void kl_cmdcc_new(struct kl_cmdcc* cmdcc) {
    for (int icc = 0; icc < CCNone; ++ icc) {
#if WINDOWS
        // TODO: use CreateProcess
        char command[64];
        if (snprintf(command, 32, "%s /help", ccs[icc]) >= 32) {
            abl_elog("unexpected error");
            exit(1);
        }
        FILE* vpipe = _popen(command, "r");
        if (vpipe == NULL) {
            abl_elog("spawn failed");
            exit(1);
        }
        
        // size_t len;
        // char* buf;
        // fseek(vpipe, 0, SEEK_END);
        // len = ftell(vpipe);
        // fseek(vpipe, 0, SEEK_SET);
        // cmdcc->version = abl_malloc((len + 1) * sizeof(cmdcc->version[0]));
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
	        // abl_ilog("found C compiler '%s'", ccs[icc]);
            cmdcc->cc = icc;
            return;
        }
    }

    abl_elog("could not find C compiler");
    exit(1);
}

int kl_cmdcc_compile(struct kl_opts* opts, struct kl_cmdcc* cmdcc, struct abl_str* src) {
    struct abl_str obj;
    abl_str_new(&obj);
    abl_str_cat(&obj, src->data);
    obj.data[obj.len - 1] = 'o';
#if WINDOWS
    abl_str_cat(&obj, "bj");
#endif
    

    struct abl_str logfilepath;
    abl_str_new(&logfilepath);
    abl_str_catf(&logfilepath, "%s/cc.log", opts->cachepath.data);
    FILE* cclog = fopen(logfilepath.data, "w+");
    if (cclog == NULL) {
        abl_elog("could not open file '%s/cc.log'", opts->cachepath.data);
        exit(1);
    }
    abl_str_del(&logfilepath);

    struct abl_vec_cstr args;
    abl_vec_cstr_new(&args);
    abl_vec_cstr_push(&args, ccopt1[cmdcc->cc]);
    abl_vec_cstr_push(&args, ccopt2[cmdcc->cc]);
    abl_vec_cstr_push(&args, ccoptobj[cmdcc->cc]);

#if WINDOWS
    // TODO: remove static path size limit
    char binopt[1024];
    int ret = snprintf(binopt, 1024, "%s%s", ccoptobjout[cmdcc->cc], obj.data);
    if (ret < 0 || ret > 1024) {
        abl_elog("unexpected error, probably because of too long path");
        exit(1);
    }
    abl_vec_cstr_push(&args, binopt);
#else
    abl_vec_cstr_push(&args, ccoptobjout[cmdcc->cc]);
    abl_vec_cstr_push(&args, obj.data);
#endif

    abl_vec_cstr_push(&args, src->data);
    abl_vec_cstr_push(&args, NULL);

    int exitstatus = abl_spawn(ccs[cmdcc->cc], args.data, cclog);
    if (exitstatus == 0) {
        // abl_ilog("succesfully compiled %s", obj.data);
    }
    else {
        abl_elog("C compilation exited with %d status. See logs saved in '%s/cc.log'", exitstatus, opts->cachepath.data);
#if DEBUG
        // struct abl_str logpath;
        // abl_str_new(&logpath);
        // abl_str_cat(&logpath, opts->cachepath.data);
        // abl_path_push(&logpath, "cc.log");
        // FILE* cclog = fopen(logpath.data, "r");
        // abl_str_del(&logpath),
        fseek(cclog, 0, SEEK_END);
        long int filesize = ftell(cclog);
        fseek(cclog, 0, SEEK_SET);
        
        char* content = abl_malloc(sizeof(char) * (filesize + 1));
        content[filesize] = '\0';
        fread(content, filesize, 1, cclog);
        fprintf(stderr, "%s\n", content);
        abl_free(content);
#endif
        kl_exit(1);
    }
    fclose(cclog);
    abl_vec_cstr_del(&args);
    abl_str_del(&obj);
    return exitstatus;
}

int kl_cmdcc_link(struct kl_opts* opts, struct kl_cmdcc* cmdcc, struct abl_vec_str* objs, struct abl_str* bin) {
    struct abl_str logfilepath;
    abl_str_new(&logfilepath);
    abl_str_catf(&logfilepath, "%s/link.log", opts->cachepath.data);
    FILE* linklog = fopen(logfilepath.data, "w+");
    if (linklog == NULL) {
        abl_elog("could not open file '%s/cc.log'", opts->cachepath.data);
        exit(1);
    }
    abl_str_del(&logfilepath);

    struct abl_vec_cstr args;
    abl_vec_cstr_new(&args);
#if WINDOWS
    // TODO: remove static path size limit
    char binopt[1024];
    int ret = snprintf(binopt, 1024, "%s%s", ccoptout[cmdcc->cc], bin->data);
    if (ret < 0 || ret > 1024) {
        abl_elog("unexpected error, probably because of too long path");
        exit(1);
    }
    abl_vec_cstr_push(&args, binopt);
#else
    abl_vec_cstr_push(&args, ccoptout[cmdcc->cc]);
    abl_vec_cstr_push(&args, bin->data);
#endif

    for (int i = 0; i < objs->size; ++ i) {
        abl_vec_cstr_push(&args, objs->data[i].data);
    }
    abl_vec_cstr_push(&args, NULL);

    int exitstatus = abl_spawn(ccs[cmdcc->cc], args.data, linklog);
    if (exitstatus) {
        abl_elog("Linking exited with %d status. See logs saved in '%s/link.log'", exitstatus, opts->cachepath.data);
#if DEBUG
        fseek(linklog, 0, SEEK_END);
        long int filesize = ftell(linklog);
        fseek(linklog, 0, SEEK_SET);
        
        char* content = abl_malloc(sizeof(char) * (filesize + 1));
        content[filesize] = '\0';
        fread(content, filesize, 1, linklog);
        fprintf(stderr, "%s\n", content);
        abl_free(content);
#endif
    }
    fclose(linklog);
    abl_vec_cstr_del(&args);
    return exitstatus;
}

void kl_cmdcc_del(struct kl_cmdcc* cmdcc) {
    (void)cmdcc;
}

int kl_cc(struct kl_opts* opts, struct abl_str* src) {
    struct kl_cmdcc cmdcc;
    kl_cmdcc_new(&cmdcc);
    int status = kl_cmdcc_compile(opts, &cmdcc, src);
    kl_cmdcc_del(&cmdcc);
    return status;
}

int kl_link(struct kl_opts* opts, struct abl_vec_str* objs, struct abl_str* bin) {
    struct kl_cmdcc cmdcc;
    kl_cmdcc_new(&cmdcc);
    int status = kl_cmdcc_link(opts, &cmdcc, objs, bin);
    kl_cmdcc_del(&cmdcc);
    return status;
}
