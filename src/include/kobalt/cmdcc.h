#ifndef CMDCC__H
#define CMDCC__H

enum kbcc {
    CCGcc = 0,
    CCClang = 1,
    CCNone = 2,
};

struct kbcmdcc {
    enum kbcc cc;
    char* version;
};

void kbcmdcc_new(struct kbcmdcc* cmdcc);

void kbcmdcc_compile(struct kbcmdcc* cmdcc, char* src, char* bin);

#endif
