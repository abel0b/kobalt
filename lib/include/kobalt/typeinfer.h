#ifndef TYPEINFER_H
#define TYPEINFER_H

#include "kobalt/kobalt.h"
#include "kobalt/ast.h"
#include "kobalt/modgraph.h"
#include "kobalt/str.h"


void kbtypeinfer(struct kbast* ast, struct kbmodgraph* modgraph, struct kbstr* modid);

#endif
