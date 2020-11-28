#ifndef KBPIPELINE__H
#define KBPIPELINE__H

#include "kobalt/queue.h"
#include "kobalt/compiland.h"
#include "kobalt/vec.h"
#include "kobalt/ast.h"
#include "kobalt/stage.h"
#include "kobalt/token.h"
#include "kobalt/options.h"
#include "kobalt/astinfo.h"

struct kbpipe {
    struct kbcompiland compiland;
    struct kbvec_token tokens;
    struct kbast ast;
    struct kbastinfo astinfo;
};

struct kbstage {
    enum kbstage_id id;
    struct kbqueue_int inputs;
};

struct kbpipeline {
    struct kbopts* opts;
    struct kbvec stages;
    struct kbvec pipes;
};

void kbpipeline_new(struct kbpipeline* pipeline, struct kbopts* opts);
void kbpipeline_push(struct kbpipeline* pipeline, struct kbcompiland* compiland);
void kbpipeline_spawn(struct kbpipeline* pipeline, enum kbstage_id stage, int pipeid);
void kbpipeline_run(struct kbpipeline* pipeline, struct kbcompiland* input);
void kbpipeline_del(struct kbpipeline* pipeline);

#endif
