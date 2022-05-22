#ifndef KLPARSER__H
#define KLPARSER__H

#include "kobalt/kobalt.h"
#include "kobalt/token.h"
#include "kobalt/syntax.h"
#include "kobalt/error.h"
#include "kobalt/compiland.h"
#include "kobalt/ast.h"

void kl_parse(struct abl_vec_token* tokens, struct kl_compiland* compiland, struct kl_ast* ast);

#endif
