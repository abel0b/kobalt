#ifndef KBPARSER__H
#define KBPARSER__H

#include "kobalt/kobalt.h"
#include "kobalt/token.h"
#include "kobalt/syntax.h"
#include "kobalt/error.h"
#include "kobalt/compiland.h"
#include "kobalt/ast.h"

void kbparse(struct kbvec_token* tokens, struct kbcompiland* compiland, struct kbast* ast);

#endif
