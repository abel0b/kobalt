#ifndef KLMODANAL__H
#define KLMODANAL__H

#include "kobalt/ast.h"
#include "kobalt/modgraph.h"
#include "abl/str.h"

void kl_modanal(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid);

#endif
