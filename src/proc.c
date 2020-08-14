#include "kobalt/proc.h"
#include <string.h>
#include <stdio.h>
#if WINDOWS
#include <process.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#endif

int kbspawn(char* argv[], FILE* stdoutlog) {
#if WINDOWS
    char command[512];
    int cur = 0;
    
    for(char** arg = argv; *arg != NULL; ++arg) {
        if (**arg == '\0') continue;
        int add = strlen(*arg) + 1;
        if (cur + add >= 512) {
            fprintf(stderr, "error: unexepected error");
            exit(1);
        }
        strcpy(&command[cur], *arg);
        cur += add;
        command[cur - 1] = ' ';
    }
    command[cur - 1] = '\0';
    fprintf(stdout, "CC %s\n", command);

    FILE* vpipe = _popen(command, "r");

    if (vpipe == NULL) {
        fprintf(stderr, "error: could not create subprocess\n");
        perror("popen");
        exit(1);
    }

    char buf[128];

    while(fgets(buf, 128, vpipe)) {
        fputs(buf, stdoutlog);
    }

    int exitstatus;
    if (feof(vpipe)) {
        exitstatus = _pclose(vpipe);
    }
    else{
        fprintf(stderr, "error: Failed to read the pipe to the end\n");
        exit(1);
    }
#else
    int pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) {
        int fd = fileno(stdoutlog);
        if (cclog == -1) {
            perror("error: open");
            exit(1);
        }
        close(STDOUT_FILENO);
        dup(fd);
        // close(STDERR_FILENO);
        // dup(fd);
        close(fd);
        execvp(argv);
    }
    else {
        int exitstatus;
        wait(&exitstatus);
    }
#endif
    return exitstatus;
}
