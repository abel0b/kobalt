#ifndef KLCGEN_H
#define KLCGEN_H

#include "kobalt/kobalt.h"
#include "kobalt/ast.h"
#include "kobalt/options.h"
#include "kobalt/compiland.h"
#include "abl/str.h"
#include "abl/vec.h"
#include "kobalt/modgraph.h"

int kl_cgen(struct kl_opts* opts, struct kl_compiland* compiland, struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid);

#endif
