#ifndef KBCGEN_H
#define KBCGEN_H

#include "kobalt/kobalt.h"
#include "kobalt/ast.h"
#include "kobalt/options.h"
#include "kobalt/source.h"
#include "kobalt/astinfo.h"
#include "kobalt/str.h"

int kbcgen(struct kbopts* opts, struct kbsrc* src, struct kbast* ast, struct kbastinfo* astinfo, struct kbstr* exe, struct kbstr* csrc);

#endif
