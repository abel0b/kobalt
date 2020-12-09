#ifndef KLPATH__H
#define KLPATH__H

#include "klbase/str.h"

void kl_path_push(struct kl_str* path, char* subpath);

void kl_path_pushf(struct kl_str* path, char* fmt, ...);

void kl_path_normalize(struct kl_str* path);

char* kl_path_basename(struct kl_str* path);

#endif
