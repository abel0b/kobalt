#include "kobalt/syntax.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

char * kbnode_kind_str(enum kbnode_kind kind) {
    switch(kind) {
        case NPROG:
            return "NPROG";
        case NITEM:
            return "NITEM";
        case NFUN:
            return "NFUN";
        case NARG:
            return "NARG";
        case NFUNBODY:
            return "NFUNBODY";
        case NDECL:
            return "NDECL";
        case NTYPE:
            return "NTYPE";
        case NEXPR:
            return "NEXPR";
        case NTERM:
            return "NTERM";
        case NBINEXPR:
            return "NBINEXPR";
        case NLIT:
            return "NLIT";
        case NFUNCALL:
            return "NFUNCALL";
        case NASSIGN:
            return "NASSIGN";
        case NID:
            return "NID";
    }
    return "UNDEFINED";
}
