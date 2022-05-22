#ifndef TYPEINFER_H
#define TYPEINFER_H

#include "kobalt/kobalt.h"
#include "kobalt/ast.h"
#include "kobalt/modgraph.h"
#include "abl/str.h"


void kl_typeinfer(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid);

#endif
