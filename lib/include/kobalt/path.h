#ifndef KBPATH__H
#define KBPATH__H

#include "kobalt/str.h"

void kbpath_push(struct kbstr* path, char* subpath);

void kbpath_pushf(struct kbstr* path, char* fmt, ...);

void kbpath_normalize(struct kbstr* path);

char* kbpath_basename(struct kbstr* path);

#endif
