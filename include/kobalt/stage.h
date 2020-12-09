#ifndef KLSTAGE__H
#define KLSTAGE__H

#define NUM_STAGES 6

enum kl_stage_id {
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

char* kl_stage_to_str(enum kl_stage_id id);

#endif
