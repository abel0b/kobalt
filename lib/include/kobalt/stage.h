#ifndef KBSTAGE__H
#define KBSTAGE__H

#define NUM_STAGES 6

enum kbstage_id {
    LexingStage = 1,
    ParsingStage = 2,
    ModAnalysisStage = 4,
    TypeInferStage = 8,
    TypeCheckStage = 16,
    CGenStage = 32,
    CCStage = 64,
    LinkStage = 128,
    ExecStage = 256,
    AllStage = 511,
};

char* kbstage_to_str(enum kbstage_id id);

#endif
