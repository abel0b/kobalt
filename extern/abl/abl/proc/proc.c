#include "abl/proc.h"
#include "abl/log.h"
#include "abl/str.h"
#include "abl/mem.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#if WINDOWS
#include <process.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#endif

ABL_API int abl_spawn(char* prog, char* args[], FILE* logfile) {
    char** argv = NULL;
    {
        int numarg = 0;
#if DEBUG
        if (getenv("DEBUG_PROC")) {
            abl_dlog("spawn %s", prog);
        }
#endif
        while(args[numarg] != NULL) {
#if DEBUG
        if (getenv("DEBUG_PROC") && args[numarg][0]) {
            abl_dlog("arg.%d = %s", numarg, args[numarg]);
        }
#endif
            numarg++;
        }
        argv = abl_malloc(sizeof(argv[0]) * (numarg + 1 + 1));
        argv[0] = prog;
        for(int i = 0; i < numarg + 1; ++i) {
            argv[i + 1] = args[i];
        }
        argv[numarg + 1] = NULL;
    }

    int exitstatus = 1;
#if WINDOWS
    struct abl_str command;
    abl_str_new(&command);

    abl_str_catf(&command, "start /b /w ");
    int cur = 0;
    for(char** arg = argv; *arg != NULL; ++arg) {
        if (**arg == '\0') {
            continue;
        }
        if (arg != argv) {
            abl_str_catf(&command, " ");
        }
        abl_str_catf(&command, "%s", *arg);
    }

    FILE* vpipe = _popen(command.data, "r");
    abl_str_del(&command);

    if (vpipe == NULL) {
        abl_elog("could not create subprocess");
        exit(1);
    }

    char buf[128];

    while(fgets(buf, 128, vpipe)) {
        if (logfile) {
            fputs(buf, logfile);
        }
        else {
            fputs(buf, stdout);
        }
    }

    if (feof(vpipe)) {
        exitstatus = _pclose(vpipe);
    }
    else{
        abl_elog("Failed to read the pipe to the end");
        exit(1);
    }
#else
    int pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) {
        if (logfile != NULL) {
            int fd = fileno(logfile);
            if (logfile != stdout) {
                close(STDOUT_FILENO);
                if (dup(fd) == -1) {
                    perror("dup");
                    exit(1);
                };
            }
            if (logfile != stderr) {
                close(STDERR_FILENO);
                dup(fd);
            }
            close(fd);
        }
        execvp(argv[0], argv);
        exit(127);
    }
    else {
        int wstatus;
        wait(&wstatus);
        if(WIFEXITED(wstatus)) {
            exitstatus = WEXITSTATUS(wstatus);
            if (exitstatus == 127) {
                abl_elog("spawn process failed");
                exit(1);
            }
        }
        else {
            if (exitstatus == 127) {
                abl_elog("spawn process failed");
                exit(1);
            }
        }

    }
#endif
    abl_free(argv);
    return exitstatus;
}
