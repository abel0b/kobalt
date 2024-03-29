#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kobalt/kobalt.h"
#include "linenoise.h"
#include "abl/log.h"

int eval(char* prog) {
    unused(prog);
    printf("val _ : foo = <bar>\n");
    return 0;
}

int kl_repl() {
    printf("Kobalt %s\n", KLVERSION);
    abl_ilog("the REPL is not currently implemented");
    const char *prompt = "> ";
    char *line;
    int loop = 1;

    linenoiseSetMultiLine(1);

    // linenoiseHistoryLoad("kobalt_history.txt");
    while(loop && ((line = linenoise(prompt)) != NULL)) {
        if (line[0] != '\0' && line[0] != '/') {
            linenoiseHistoryAdd(line);
            // linenoiseHistorySave("kobalt_history.txt");
            eval(line);
        }
        else if (!strncmp(line,"/historylen", 11)) {
            int len = atoi(line + 11);
            linenoiseHistorySetMaxLen(len);
        }
        else if (!strncmp(line, "/exit", 5)) {
            loop = 0;
        }
        else if (line[0] == '/') {
            printf("error: unreconized command '%s'\n", line);
        }
        free(line);
    }
    return 0;
}
