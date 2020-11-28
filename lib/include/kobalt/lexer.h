#ifndef KBLEXER__H
#define KBLEXER__H

#include "kobalt/kobalt.h"
#include "kobalt/compiland.h"
#include "kobalt/token.h"
#include "kobalt/vec.h"

extern char * keywords[];

void kblex(struct kbcompiland* src, struct kbvec_token* tokens);

#endif
