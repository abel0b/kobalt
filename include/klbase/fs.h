#ifndef KLFS__H
#define KLFS__H

#include "kobalt/kobalt.h"
#include "klbase/str.h"

void ensuredir(char* path);

int isds(char c);

void read_to_str(char* path, struct kl_str* dest);

#endif
