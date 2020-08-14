#include "kobalt/uid.h"
#include <stdio.h>

static uint64_t state = 1;

void seed(uint64_t s) {
    state = s;
}

uint64_t genuint64() {
    uint64_t x = state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return state = x;
}

static void genuid_aux(char uid[8], char alphabet[36]) {
    uint64_t num = genuint64();
    unsigned char* buf = (unsigned char*)&num;
    for(int ii=0; ii<8; ++ii) {
        uid[ii] = alphabet[(int)(buf[ii] / 7.28)];
    }
}

void genuid(char uid[8]) {
    genuid_aux(uid, "abcdefghijklmnopqrstuvwxyz0123456789");
}

void genuidmaj(char uid[8]) {
    genuid_aux(uid, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
}
