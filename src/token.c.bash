set -f

cat << END
#include "kobalt/token.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char * specials[NUM_SPECIALS] = {
    "",
END

while read -r -s line
do
    token=($line)
    echo "    \"${token[1]}\","
done < src/token.csv

cat << END
    "",
};

struct kbtoken kbtoken_make(enum kbtoken_kind kind, char * value, int line, int col) {
    struct kbtoken token;
    token.kind = kind;
    if (value == NULL) {
        token.value = NULL;
    }
    else {
        token.value = malloc(strlen(value)+1);
        strcpy(token.value, value);
    }
    token.line = line;
    token.col = col;
    return token;
}

void kbtoken_del_arr(unsigned int num_tokens, struct kbtoken * tokens) {
    for(unsigned int ii=0; ii<num_tokens; ii++) {
        if(tokens[ii].value != NULL) free(tokens[ii].value);
    }
    free(tokens);
}

void kbtoken_display(struct kbtoken * token) {
    printf("%s", kbtoken_string(token->kind));
    if (token->value != NULL) {
        printf("=\"%s\"", token->value);
    }
    printf(" at %d:%d\\n", token->line, token->col);
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
done < src/token.csv

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
