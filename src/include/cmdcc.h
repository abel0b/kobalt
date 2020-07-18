#ifndef CMDCC__H
#define CMDCC__H

struct kbcmdcc {
    char * cc;
};

void kbcmdcc_new(struct kbcmdcc * cmdcc);

void kbcmdcc_compile(struct kbcmdcc* cmdcc, char* src, char* bin);

#endif
