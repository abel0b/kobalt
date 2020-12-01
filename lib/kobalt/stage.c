#include "kobalt/stage.h"

char* kbstage_to_str(enum kbstage_id id) {
    switch(id) {
        case LexingStage:
            return "lexing";
        case ParsingStage:
            return "parsing";
        case ModAnalysisStage:
            return "modanalysis";
        case TypeInferStage:
            return "typeinfer";
        case TypeCheckStage:
            return "typecheck";
        case CGenStage:
            return "cgen";
        case CCStage:
            return "cc";
        case LinkStage:
            return "link";
        case ExecStage:
            return "exec";
        case AllStage:
            return "all";
    }
}
