#ifndef TYPECHECK_H
#define TYPECHECK_H

#include "kobalt/kobalt.h"
#include "kobalt/ast.h"
#include "kobalt/modgraph.h"
#include "klbase/str.h"

void kl_typecheck(struct kl_ast* ast, struct kl_modgraph* modgraph, struct kl_str* modid);

#endif
