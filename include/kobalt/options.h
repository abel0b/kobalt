#ifndef KBOPTIONS__H
#define KBOPTIONS__H

struct kbopts {
    char * cwd;
    int num_srcs;
    struct kbsrc * srcs;
};

struct kbopts kbopts_make(int argc, char * argv[]);

void kbopts_destroy(struct kbopts * options);

#endif
