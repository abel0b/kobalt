#ifndef KBUID__H
#define KBUID__H

#include "kobalt/kobalt.h"
#include <stdint.h>

void seed(uint64_t s);

uint64_t genuint64();

void genuid(char uid[8]);

void genuidmaj(char uid[8]);

#endif
