set -f

cat << END
#include "kobalt/syntax.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char* kl_node_kind_str(enum kl_node_kind kind) {
    switch(kind) {
END

while read -r -s line
do
    tok=($line)
    cat << END
        case N${tok[0]}:
            return "${tok[0]}";
END
done < lib/kobalt/syntax.csv

cat << END
    }
    return "UNDEFINED";
}

int isgroup(struct kl_node *node) {
    return node->kind == NProgram || node->kind == NFunParams || node->kind == NSeq || node->kind == NCallParams || node->kind == NIfElse || node->kind == NCase;
}
END
