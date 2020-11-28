#ifndef KBSTAGE__H
#define KBSTAGE__H

#define NUM_STAGES 6

enum kbstage_id {
    LexingStage = 1,
    ParsingStage = 2,
    TypeInferStage = 4,
    TypeCheckStage = 8,
    CGenStage = 16,
    CCStage = 32,
    LinkStage = 64,
    ExecStage = 128,
    AllStage = 255,
};

#endif
