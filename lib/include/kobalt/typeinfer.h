#ifndef TYPEINFER_H
#define TYPEINFER_H

#include "kobalt/kobalt.h"
#include "kobalt/ast.h"
#include "kobalt/astinfo.h"

void kbtypeinfer(struct kbast* ast, struct kbastinfo* astinfo);

#endif
