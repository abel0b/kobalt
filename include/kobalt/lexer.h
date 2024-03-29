#ifndef KLLEXER__H
#define KLLEXER__H

#include "kobalt/kobalt.h"
#include "kobalt/compiland.h"
#include "kobalt/token.h"
#include "abl/vec.h"

extern char * keywords[];

void kl_lex(struct kl_compiland* src, struct abl_vec_token* tokens);

#endif
