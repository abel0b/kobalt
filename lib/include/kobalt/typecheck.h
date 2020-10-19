#ifndef TYPECHECK_H
#define TYPECHECK_H

#include "kobalt/kobalt.h"
#include "kobalt/syntax.h"
#include "kobalt/ast.h"
#include "kobalt/vec.h"

struct kbtyping {
    struct kbvec constraints;
};

void kbtypecheck(struct kbast* ast);

#endif
