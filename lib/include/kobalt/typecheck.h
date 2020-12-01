#ifndef TYPECHECK_H
#define TYPECHECK_H

#include "kobalt/kobalt.h"
#include "kobalt/ast.h"
#include "kobalt/modgraph.h"
#include "kobalt/str.h"

void kbtypecheck(struct kbast* ast, struct kbmodgraph* modgraph, struct kbstr* modid);

#endif
