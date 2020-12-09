#include "kobalt/pipeline.h"
#include "klbase/log.h"
#include "kobalt/lexer.h"
#include "kobalt/parser.h"
#include "kobalt/typeinfer.h"
#include "kobalt/typecheck.h"
#include "kobalt/cgen.h"
#include "kobalt/token.h"
#include "klbase/proc.h"
#include "kobalt/cctool.h"
#include "kobalt/modanal.h"
#include "kobalt/stdkl.h"
#include <string.h>
#include <stdbool.h>

void kl_pipeline_new(struct kl_pipeline* pipeline, struct kl_opts* opts) {
    kl_vec_new(&pipeline->stages, sizeof(struct kl_stage));
    kl_vec_new(&pipeline->pipes, sizeof(struct kl_pipe));
    kl_modgraph_new(&pipeline->modgraph);
    pipeline->opts = opts;
#define addstage(K) do {\
    if (pipeline->opts->stages & K) {\
        struct kl_stage stage;\
        stage.id = K;\
        kl_queue_int_new(&stage.inputs);\
        kl_vec_push(&pipeline->stages, &stage);\
    }\
} while(0)

    addstage(LexingStage);
    addstage(ParsingStage);
    addstage(ModAnalysisStage);
    addstage(TypeInferStage);
    addstage(TypeCheckStage);
    addstage(CGenStage);
    addstage(CCStage);
    addstage(ExecStage);

    if ((pipeline->opts->stages | AllStage) ^ AllStage) {
        kl_wlog("ignoring unknown pipeline stage %d", (pipeline->opts->stages | AllStage) ^ AllStage);
    }
}

void kl_pipeline_push(struct kl_pipeline* pipeline, struct kl_compiland* compiland) {
    kl_vec_resize(&pipeline->pipes, pipeline->pipes.size + 1);
    struct kl_pipe* pipe = (struct kl_pipe*) kl_vec_last(&pipeline->pipes);
    memcpy(&pipe->compiland, compiland, sizeof(*compiland));
    kl_pipeline_spawn(pipeline, LexingStage, pipeline->pipes.size - 1);
}

void kl_pipeline_spawn(struct kl_pipeline* pipeline, enum kl_stage_id id, int pipeid) {
    for(int i = 0; i < pipeline->stages.size; ++ i) {
        struct kl_stage* stage = kl_vec_get(&pipeline->stages, i);
        if (stage->id == id) {
            kl_queue_int_enqueue(&stage->inputs, pipeid);
        }
    }
    if ((id | AllStage) ^ AllStage) {
        kl_wlog("ignoring unknown pipeline stage %d", (id | AllStage) ^ AllStage);
    }
}

static void pipeline_run_stage(struct kl_pipeline* pipeline, enum kl_stage_id id, int pipeid) {
    struct kl_pipe* pipe = (struct kl_pipe*) kl_vec_get(&pipeline->pipes, pipeid);
#if DEBUG
    if (getenv("DEBUG_PIPELINE")) {
        kl_dlog("stage %s : %s", kl_stage_to_str(id), pipe->compiland.name.data);
    }
#endif

    #define spawn_if(S) if (pipeline->opts->stages & S) {\
        kl_pipeline_spawn(pipeline, S, pipeid);\
    }
    switch(id) {
        case LexingStage:
            kl_lex(&pipe->compiland, &pipe->tokens);
            spawn_if(ParsingStage);
            break;
        case ParsingStage:
            kl_parse(&pipe->tokens, &pipe->compiland, &pipe->ast);
            spawn_if(ModAnalysisStage);
            break;
        case ModAnalysisStage:
            {
                struct kl_mod* mod = kl_modgraph_add(&pipeline->modgraph, &pipe->compiland.name, &pipe->ast);
                kl_modanal(&pipe->ast, &pipeline->modgraph, &pipe->compiland.name);
                
                // kl_elog("dep %s %d", pipe->compiland.path.data, mod->deps.size, mod->deps.data[0]);
                // exit(1);
                for (int i = 0; i < mod->deps.size; ++ i) {
                    if (!kl_modgraph_try_get(&pipeline->modgraph, &mod->deps.data[i])) {
                        struct kl_compiland compiland;
                        if (strcmp(mod->deps.data[i].data, "std") == 0) {
                            kl_compiland_new_virt(&compiland, "std.kl", stdkl);
                        }
                        else {
                            kl_elog("module import is not yet implemented");
                            exit(1);
                        }
                        kl_pipeline_push(pipeline, &compiland);
                    }
                }
                spawn_if(TypeInferStage);
            }
            break;
        case TypeInferStage:
            kl_typeinfer(&pipe->ast, &pipeline->modgraph, &pipe->compiland.name);
            spawn_if(TypeCheckStage);
            break;
        case TypeCheckStage:
            kl_typecheck(&pipe->ast, &pipeline->modgraph, &pipe->compiland.name);
            spawn_if(CGenStage);
            break;
        case CGenStage:
            kl_cgen(pipeline->opts, &pipe->compiland, &pipe->ast, &pipeline->modgraph, &pipe->compiland.name);
            spawn_if(CCStage);
            break;
        case CCStage:
            {
                struct kl_str cfile;
                kl_str_new(&cfile);
                kl_str_cat(&cfile, pipeline->opts->cachepath.data);
                kl_str_catf(&cfile, "/%s.c", pipe->compiland.name);
                int status = kl_cc(pipeline->opts, &cfile);
                unused(status);
                kl_str_del(&cfile);
            }
            break;
        case ExecStage:
            break;
        default:
            kl_elog("unexpected pipeline stage %d", id);
            exit(1);
            break;
    }
}

void kl_pipeline_run(struct kl_pipeline* pipeline, struct kl_compiland* input) {
    kl_pipeline_push(pipeline, input);
    for(int i = 0; i < pipeline->stages.size; ++ i) {
        bool done = false;
        do {
            done = true;
            for(int j = 0; j <= i; ++ j) {
                struct kl_stage* stage = kl_vec_get(&pipeline->stages, j);
                while(stage->inputs.size) {
                    done = false;   
                    int pipe = kl_queue_int_dequeue(&stage->inputs);
                    pipeline_run_stage(pipeline, stage->id, pipe);
                }
            }
        } while(!done);
    }

    FILE* out = (pipeline->opts->outpath.len)? fopen(pipeline->opts->outpath.data, "w") : stdout;
    if (out == NULL) {
        kl_elog("could not open output file '%s'", pipeline->opts->outpath.data);
        exit(1);
    }
    

    if ((pipeline->opts->stages & LexingStage) && (pipeline->opts->stages >> 1 == 0)) {
        for(int i = 0; i < pipeline->pipes.size; ++ i) {
            struct kl_pipe* pipe = (struct kl_pipe*) kl_vec_get(&pipeline->pipes, i);
            for(int j = 0; j < pipe->tokens.size; ++ j) {
                kl_token_display(out, &pipe->tokens.data[j]);
            }
        }
    }

    if ((pipeline->opts->stages & ParsingStage) && (pipeline->opts->stages >> 2 == 0)) {
        for(int i = 0; i < pipeline->pipes.size; ++ i) {
            struct kl_pipe* pipe = (struct kl_pipe*) kl_vec_get(&pipeline->pipes, i);
            kl_ast_display(out, &pipe->ast, NULL);
        }
    }

    if ((pipeline->opts->stages & ModAnalysisStage) && (pipeline->opts->stages >> 4 == 0)) {
        for(int i = 0; i < pipeline->pipes.size; ++ i) {
            struct kl_pipe* pipe = (struct kl_pipe*) kl_vec_get(&pipeline->pipes, i);
            kl_ast_display(out, &pipe->ast, NULL);
        }
    }

    if ((pipeline->opts->stages & (TypeInferStage | TypeCheckStage)) && (pipeline->opts->stages >> 8 == 0)) {
        for(int i = 0; i < pipeline->pipes.size; ++ i) {
            struct kl_pipe* pipe = (struct kl_pipe*) kl_vec_get(&pipeline->pipes, i);
            struct kl_mod* mod = (struct kl_mod*) kl_modgraph_get(&pipeline->modgraph, &pipe->compiland.name);
            kl_ast_display(out, &pipe->ast, &mod->astinfo);
        }
    }

    if (pipeline->opts->stages & CCStage) {
        struct kl_vec_str objs;
        kl_vec_str_new(&objs);
        fprintf(pipeline->opts->manifest, "csrc:\n");
        for(int j = 0; j < pipeline->pipes.size; ++ j) {
            struct kl_pipe* pipe = (struct kl_pipe*) kl_vec_get(&pipeline->pipes, j);
            struct kl_str obj;
            kl_str_new(&obj);
#if WINDOWS
            kl_str_catf(&obj, "%s/%s.obj", pipeline->opts->cachepath.data, pipe->compiland.name.data);
#else
            kl_str_catf(&obj, "%s/%s.o", pipeline->opts->cachepath.data, pipe->compiland.name.data);
#endif
            kl_vec_str_push(&objs, obj);
            fprintf(pipeline->opts->manifest, "  - %s.c\n", pipe->compiland.name.data);
        }
        struct kl_str bin;
        kl_str_new(&bin);
        kl_str_catf(&bin, "%s/%s", pipeline->opts->cachepath.data, input->name.data);

        kl_link(pipeline->opts, &objs, &bin);

        if (pipeline->opts->stages & ExecStage) {
            kl_spawn(bin.data, (char**) pipeline->opts->exe_argv.data, NULL);
        }

        kl_str_del(&bin);
        for(int j = 0; j < objs.size; ++ j) {
            kl_str_del(&objs.data[j]);
        }
        kl_vec_str_del(&objs);
    }

    if (pipeline->opts->outpath.len) {
        fclose(out);
    }
}

void kl_pipeline_del(struct kl_pipeline* pipeline) {
    for(int i = 0; i < pipeline->stages.size; ++ i) {
        struct kl_stage* stage = (struct kl_stage*) kl_vec_get(&pipeline->stages, i);
        kl_queue_int_del(&stage->inputs);
    }
    kl_vec_del(&pipeline->stages);
    for(int i = 0; i < pipeline->pipes.size; ++ i) {
        struct kl_pipe* pipe = (struct kl_pipe*) kl_vec_get(&pipeline->pipes, i);
        kl_compiland_del(&pipe->compiland);
        if (pipeline->opts->stages & LexingStage) {
            for(int j = 0; j < pipe->tokens.size; ++ j) {
                kl_token_del(&pipe->tokens.data[j]);
            }
            kl_vec_token_del(&pipe->tokens);
        }
        if (pipeline->opts->stages & ParsingStage) {
            kl_ast_del(&pipe->ast);
        }
        if (pipeline->opts->stages & CGenStage) {

        }
    }
    kl_vec_del(&pipeline->pipes);
    kl_modgraph_del(&pipeline->modgraph);
}
