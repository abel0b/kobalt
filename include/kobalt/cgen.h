#ifndef KLCGEN_H
#define KLCGEN_H

#include "kobalt/kobalt.h"
#include "kobalt/ast.h"
#include "kobalt/options.h"
#include "kobalt/compiland.h"
#include "klbase/str.h"
#include "klbase/vec.h"
#include "kobalt/modgraph.h"

int kl_cgen(struct kl_opts* opts, struct kl_compiland* compiland, struct kl_ast* ast, struct kl_modgraph* modgraph, struct kl_str* modid);

#endif
