#ifndef KLMODANAL__H
#define KLMODANAL__H

#include "kobalt/ast.h"
#include "kobalt/modgraph.h"
#include "klbase/str.h"

void kl_modanal(struct kl_ast* ast, struct kl_modgraph* modgraph, struct kl_str* modid);

#endif
