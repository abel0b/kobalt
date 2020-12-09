#include "klbase/uid.h"
#include "klbase/time.h"
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

static void genuid_aux(char uid[8], char alphabet[16]) {
    uint64_t num = kl_time_get_us();
    unsigned char* buf = (unsigned char*)&num;
    for(int i = 0; i < 8; ++i) {
        uid[i] = alphabet[(int)buf[7 - i] / 16];
    }
}

void genuid(char uid[8]) {
    genuid_aux(uid, "0123456789abcdef");
}

void genuidmaj(char uid[8]) {
    genuid_aux(uid, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
}
