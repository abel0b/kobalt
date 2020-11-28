#include <stdio.h>
#include <stdlib.h>
#include "kobalt/options.h"
#include "kobalt/pipeline.h"
#include "repl.h"
#include <stdlib.h>

#if UNIX
#if defined(__GLIBC__)
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

void handler(int sig) {
    void* array[10];
    size_t size;

    size = backtrace(array, 10);

    fprintf(stderr, "error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}
#endif
#endif

int main(int argc, char * argv[]) {
#if UNIX
#if defined(__GLIBC__)
    signal(SIGABRT, handler);
#endif
#endif

    if (argc == 1) {
        return kb_repl();
    }

    int status = EXIT_SUCCESS;

    struct kbopts opts;
    kbopts_new(&opts, argc, argv);

    struct kbpipeline pipeline;
    kbpipeline_new(&pipeline, &opts);

    for(int i = 0; i < opts.inputs.size; ++ i) {
        struct kbcompiland* input = (struct kbcompiland*) kbvec_get(&opts.inputs, i);
        kbpipeline_run(&pipeline, input);
    }
    
    kbpipeline_del(&pipeline);

    kbopts_del(&opts);
    return status;
}
