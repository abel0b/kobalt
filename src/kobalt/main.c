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
    size_t size = backtrace(array, 10);
    fprintf(stderr, "error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}
void onexit() {
    abort();
}
#endif
#endif

int main(int argc, char * argv[]) {
#if UNIX
#if defined(__GLIBC__)
    signal(SIGABRT, handler);
    // atexit(onexit);
#endif
#endif

    if (argc == 1) {
        return kl_repl();
    }

    int status = EXIT_SUCCESS;

    struct kl_opts opts;
    kl_opts_new(&opts, argc, argv);

    struct kl_pipeline pipeline;
    kl_pipeline_new(&pipeline, &opts);

    for(int i = 0; i < opts.inputs.size; ++ i) {
        struct kl_compiland* input = (struct kl_compiland*) kl_vec_get(&opts.inputs, i);
        kl_pipeline_run(&pipeline, input);
    }
    
    kl_pipeline_del(&pipeline);

    kl_opts_del(&opts);
    return status;
}
