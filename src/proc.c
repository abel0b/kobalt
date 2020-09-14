#define _POSIX_C_SOURCE 1
#include "kobalt/proc.h"
#include "kobalt/log.h"
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

int kbspawn(char* argv[], FILE* logfile) {
    int exitstatus = 1;
#if WINDOWS
    char command[512];
    int cur = 0;
    
    for(char** arg = argv; *arg != NULL; ++arg) {
        if (**arg == '\0') continue;
        int add = strlen(*arg) + 1;
        if (cur + add >= 512) {
            kbelog("unexepected error");
            exit(1);
        }
        strcpy(&command[cur], *arg);
        cur += add;
        command[cur - 1] = ' ';
    }
    command[cur - 1] = '\0';
    FILE* vpipe = _popen(command, "r");

    if (vpipe == NULL) {
        kbelog("could not create subprocess");
        exit(1);
    }

    char buf[128];

    while(fgets(buf, 128, vpipe)) {
        fputs(buf, logfile);
    }

    if (feof(vpipe)) {
        exitstatus = _pclose(vpipe);
    }
    else{
        kbelog("Failed to read the pipe to the end");
        exit(1);
    }
#else
    int pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) {
        int fd = fileno(logfile);
        close(STDOUT_FILENO);
        if (dup(fd) == -1) {
            perror("dup");
            exit(1);
        };
        close(STDERR_FILENO);
        dup(fd);
        close(fd);
        execvp(argv[0], argv);
    }
    else {
        wait(&exitstatus);
    }
#endif
    return exitstatus;
}
