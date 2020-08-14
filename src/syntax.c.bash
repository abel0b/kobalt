set -f

cat << END
#include "kobalt/syntax.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char * kbnode_kind_str(enum kbnode_kind kind) {
    switch(kind) {
END

while read -r -s line
do
    tok=($line)
    cat << END
        case N${tok[0]}:
            return "${tok[0]}";
END
done < src/syntax.csv
cat << END
    }
    return "UNDEFINED";
}

int isgroup(struct kbnode *node) {
    return node->kind == NFile || node->kind == NFunParams || node->kind == NFunBody || node->kind == NCallParams;
}
END
