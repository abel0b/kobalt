set -f

cat << END
#include "kobalt/token.h"
#include "kobalt/memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

kbvec_impl(struct kbtoken, token)

char * specials[NUM_SPECIALS] = {
    "",
END

while read -r -s line
do
    token=($line)
    echo "    \"${token[1]}\","
done < lib/kobalt/token.csv

cat << END
    "",
};

int is_builtin_fun(enum kbtoken_kind tok_kind) {
    return (tok_kind == TPlus) || (tok_kind == TDash) || (tok_kind == TStar) || (tok_kind == TSlash) || (tok_kind == TPercent) || (tok_kind == TCaret) || (tok_kind == TExclam) || (tok_kind == TQuery) || (tok_kind == TAnd) || (tok_kind == TOr) || (tok_kind == TAndAnd) || (tok_kind == TLShift) || (tok_kind == TRShift) || (tok_kind == TEqEq) || (tok_kind == TPlusEq) || (tok_kind == TDashEq) || (tok_kind == TStarEq) || (tok_kind == TSlashEq) || (tok_kind == TPercentEq) || (tok_kind == TCaretEq) || (tok_kind == TAndEq) || (tok_kind == TOrEq)|| (tok_kind == TLShiftEq)|| (tok_kind == TRShiftEq)|| (tok_kind == TExclamEq)|| (tok_kind == TGT) || (tok_kind == TLT) || (tok_kind == TGEq) || (tok_kind == TLEq) || (tok_kind == TAt);
}

struct kbtoken kbtoken_make(enum kbtoken_kind kind, char * value, int line, int col) {
    struct kbtoken token;
    token.kind = kind;
    if (value == NULL) {
        token.value = NULL;
    }
    else {
        token.value = kbmalloc(strlen(value) + 1);
        strcpy(token.value, value);
    }
    token.loc.line = line;
    token.loc.col = col;
    return token;
}

void kbtoken_del_arr(unsigned int num_tokens, struct kbtoken * tokens) {
    for(unsigned int ii=0; ii<num_tokens; ii++) {
        if(tokens[ii].value != NULL) free(tokens[ii].value);
    }
    free(tokens);
}

void kbtoken_display(FILE* file, struct kbtoken* token) {
    fprintf(file, "%s", kbtoken_string(token->kind));
    if (token->value != NULL) {
        fprintf(file, "=\"%s\"", token->value);
    }
    fprintf(file, " at %d:%d\\n", token->loc.line, token->loc.col);
}

void kbtoken_del(struct kbtoken * token) {
    if (token->value != NULL)
        free (token->value);
}

char * kbtoken_string(enum kbtoken_kind kind) {
    switch(kind) {
        case TNL:
            return "NL";
END

declare -A seps

function parse_line {
    cat << END
        case T$1:
            return "$1";
END
    if [[ ${#2} > 0 ]]  
    then
        for ((i=0; i<${#2}; i++))
        do
            seps["${2:$i:1}"]=1
        done
    fi
}

while read -r -s line
do
    parse_line $line 
done < lib/kobalt/token.csv

cat << END
        case TILLEGAL:
            return "ILLEGAL";
    }
    return "UNDEFINED";
}

int is_delim(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n';
}

END

echo "int is_sep(char c) {"
first=1
for sep in "${!seps[@]}"
do
    if [[ $first = 1 ]]
    then
        echo "    return c == '$sep'"
        first=0
    else
        echo "        || c == '$sep'"
    fi
done

cat << END
    ;
}
END
