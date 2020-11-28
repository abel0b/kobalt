#include "kobalt/pipeline.h"
#include "kobalt/log.h"
#include "kobalt/lexer.h"
#include "kobalt/parser.h"
#include "kobalt/typeinfer.h"
#include "kobalt/typecheck.h"
#include "kobalt/cgen.h"
#include "kobalt/token.h"
#include "kobalt/proc.h"
#include "kobalt/cctool.h"
#include <string.h>
#include <stdbool.h>

void kbpipeline_new(struct kbpipeline* pipeline, struct kbopts* opts) {
    kbvec_new(&pipeline->stages, sizeof(struct kbstage));
    kbvec_new(&pipeline->pipes, sizeof(struct kbpipe));
    pipeline->opts = opts;
#define addstage(K) do {\
    if (pipeline->opts->stages & K) {\
        struct kbstage stage;\
        stage.id = K;\
        kbqueue_int_new(&stage.inputs);\
        kbvec_push(&pipeline->stages, &stage);\
    }\
} while(0)

    addstage(LexingStage);
    addstage(ParsingStage);
    addstage(TypeInferStage);
    addstage(TypeCheckStage);
    addstage(CGenStage);
    addstage(CCStage);
    addstage(ExecStage);

    if ((pipeline->opts->stages | AllStage) ^ AllStage) {
        kbwlog("ignoring unknown pipeline stage %d", (pipeline->opts->stages | AllStage) ^ AllStage);
    }
}

void kbpipeline_push(struct kbpipeline* pipeline, struct kbcompiland* compiland) {
    kbvec_resize(&pipeline->pipes, pipeline->pipes.size + 1);
    struct kbpipe* pipe = (struct kbpipe*) kbvec_last(&pipeline->pipes);
    memcpy(&pipe->compiland, compiland, sizeof(*compiland));
    kbpipeline_spawn(pipeline, LexingStage, 0);
}

void kbpipeline_spawn(struct kbpipeline* pipeline, enum kbstage_id id, int pipeid) {
    for(int i = 0; i < pipeline->stages.size; ++ i) {
        struct kbstage* stage = kbvec_get(&pipeline->stages, i);
        if (stage->id == id) {
            kbqueue_int_enqueue(&stage->inputs, pipeid);
        }
    }
    if ((id | AllStage) ^ AllStage) {
        kbwlog("ignoring unknown pipeline stage %d", (id | AllStage) ^ AllStage);
    }
}

static void pipeline_run_stage(struct kbpipeline* pipeline, enum kbstage_id id, int pipeid) {
    struct kbpipe* pipe = (struct kbpipe*) kbvec_get(&pipeline->pipes, pipeid);
    #define spawn_if(S) if (pipeline->opts->stages & S) {\
        kbpipeline_spawn(pipeline, S, pipeid);\
    }
    switch(id) {
        case LexingStage:
            kblex(&pipe->compiland, &pipe->tokens);
            spawn_if(ParsingStage);
            break;
        case ParsingStage:
            kbparse(&pipe->tokens, &pipe->compiland, &pipe->ast);
            spawn_if(TypeInferStage);
            break;
        case TypeInferStage:
            kbtypeinfer(&pipe->ast, &pipe->astinfo);
            spawn_if(TypeCheckStage);
            break;
        case TypeCheckStage:
            kbtypecheck(&pipe->ast, &pipe->astinfo);
            spawn_if(CGenStage);
            break;
        case CGenStage:
            kbcgen(pipeline->opts, &pipe->compiland, &pipe->ast, &pipe->astinfo);
            spawn_if(CCStage);
            break;
        case CCStage:
            {
                struct kbstr cfile;
                kbstr_new(&cfile);
                kbstr_cat(&cfile, pipeline->opts->cachepath.data);
                kbstr_catf(&cfile, "/%s.c", pipe->compiland.name);
                kbilog("CC %s", cfile.data);
                int status = kbcc(pipeline->opts, &cfile);
                unused(status);
                kbstr_del(&cfile);
            }
            break;
        case ExecStage:
            break;
        default:
            kbelog("unexpected pipeline stage %d", id);
            exit(1);
            break;
    }
}

void kbpipeline_run(struct kbpipeline* pipeline, struct kbcompiland* input) {
    kbpipeline_push(pipeline, input);
    bool done = false;
    while (!done) {
        done = true;
        for(int i = 0; i < pipeline->stages.size; ++ i) {
            struct kbstage* stage = kbvec_get(&pipeline->stages, i);
            while(stage->inputs.size) {
                done = false;   
                int pipe = kbqueue_int_dequeue(&stage->inputs);
                pipeline_run_stage(pipeline, stage->id, pipe);
            }
        }
    }

    FILE* out = (pipeline->opts->outpath.len)? fopen(pipeline->opts->outpath.data, "w") : stdout;
    if (out == NULL) {
        kbelog("could not open output file '%s'", pipeline->opts->outpath.data);
        exit(1);
    }
    

    if ((pipeline->opts->stages & LexingStage) && (pipeline->opts->stages >> 1 == 0)) {
        for(int i = 0; i < pipeline->pipes.size; ++ i) {
            struct kbpipe* pipe = (struct kbpipe*) kbvec_get(&pipeline->pipes, i);
            for(int j = 0; j < pipe->tokens.size; ++ j) {
                kbtoken_display(out, &pipe->tokens.data[j]);
            }
        }
    }

    if ((pipeline->opts->stages & ParsingStage) && (pipeline->opts->stages >> 2 == 0)) {
        for(int i = 0; i < pipeline->pipes.size; ++ i) {
            struct kbpipe* pipe = (struct kbpipe*) kbvec_get(&pipeline->pipes, i);
            kbast_display(out, &pipe->ast, NULL);
        }
    }

    if ((pipeline->opts->stages & (TypeInferStage | TypeCheckStage)) && (pipeline->opts->stages >> 8 == 0)) {
        for(int i = 0; i < pipeline->pipes.size; ++ i) {
            struct kbpipe* pipe = (struct kbpipe*) kbvec_get(&pipeline->pipes, i);
            kbast_display(out, &pipe->ast, &pipe->astinfo);
        }
    }

    if (pipeline->opts->stages & CCStage) {
        struct kbvec_str objs;
        kbvec_str_new(&objs);
        for(int j = 0; j < pipeline->pipes.size; ++ j) {
            struct kbpipe* pipe = (struct kbpipe*) kbvec_get(&pipeline->pipes, j);
            struct kbstr obj;
            kbstr_new(&obj);
            kbstr_catf(&obj, "%s/%s.o", pipeline->opts->cachepath.data, pipe->compiland.name);
            kbvec_str_push(&objs, obj);
        }
        struct kbstr bin;
        kbstr_new(&bin);
        kbstr_catf(&bin, "%s/%s", pipeline->opts->cachepath.data, input->name.data);

        kblink(pipeline->opts, &objs, &bin);

        if (pipeline->opts->stages & ExecStage) {
            kbspawn(bin.data, (char**) pipeline->opts->exe_argv.data, NULL);
        }

        kbstr_del(&bin);
        for(int j = 0; j < objs.size; ++ j) {
            kbstr_del(&objs.data[j]);
        }
        kbvec_str_del(&objs);
    }

    if (pipeline->opts->outpath.len) {
        fclose(out);
    }
}

void kbpipeline_del(struct kbpipeline* pipeline) {
    for(int i = 0; i < pipeline->stages.size; ++ i) {
        struct kbstage* stage = (struct kbstage*) kbvec_get(&pipeline->stages, i);
        kbqueue_int_del(&stage->inputs);
    }
    kbvec_del(&pipeline->stages);
    for(int i = 0; i < pipeline->pipes.size; ++ i) {
        struct kbpipe* pipe = (struct kbpipe*) kbvec_get(&pipeline->pipes, i);
        kbcompiland_del(&pipe->compiland);
        if (pipeline->opts->stages & LexingStage) {
            for(int j = 0; j < pipe->tokens.size; ++ j) {
                kbtoken_del(&pipe->tokens.data[j]);
            }
            kbvec_token_del(&pipe->tokens);
        }
        if (pipeline->opts->stages & ParsingStage) {
            kbast_del(&pipe->ast);
        }
        if (pipeline->opts->stages & (TypeInferStage | TypeCheckStage)) {
            kbastinfo_del(&pipe->astinfo);
        }
        if (pipeline->opts->stages & CGenStage) {

        }
    }
    kbvec_del(&pipeline->pipes);
}
