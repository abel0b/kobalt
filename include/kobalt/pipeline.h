#ifndef KLPIPELINE__H
#define KLPIPELINE__H

#include "abl/queue.h"
#include "kobalt/compiland.h"
#include "abl/vec.h"
#include "kobalt/ast.h"
#include "kobalt/stage.h"
#include "kobalt/token.h"
#include "kobalt/options.h"
#include "kobalt/modgraph.h"

struct kl_pipe {
    struct kl_compiland compiland;
    struct abl_vec_token tokens;
    struct kl_ast ast;
};

struct kl_stage {
    enum kl_stage_id id;
    struct abl_queue_int inputs;
};

struct kl_pipeline {
    struct kl_modgraph modgraph;
    struct kl_opts* opts;
    struct abl_vec stages;
    struct abl_vec pipes;
};

void kl_pipeline_new(struct kl_pipeline* pipeline, struct kl_opts* opts);
void kl_pipeline_push(struct kl_pipeline* pipeline, struct kl_compiland* compiland);
void kl_pipeline_spawn(struct kl_pipeline* pipeline, enum kl_stage_id stage, int pipeid);
void kl_pipeline_run(struct kl_pipeline* pipeline, struct kl_compiland* input);
void kl_pipeline_del(struct kl_pipeline* pipeline);

#endif
