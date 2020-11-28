#ifndef KBCGEN_H
#define KBCGEN_H

#include "kobalt/kobalt.h"
#include "kobalt/ast.h"
#include "kobalt/options.h"
#include "kobalt/compiland.h"
#include "kobalt/astinfo.h"
#include "kobalt/str.h"
#include "kobalt/vec.h"

int kbcgen(struct kbopts* opts, struct kbcompiland* compiland, struct kbast* ast, struct kbastinfo* astinfo);

#endif
