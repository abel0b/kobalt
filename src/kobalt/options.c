#include "kobalt/options.h"
#include "kobalt/compiland.h"
#include "abl/mem.h"
#include "abl/log.h"
#include "abl/fs.h"
#include "abl/uid.h"
#include "abl/time.h"
#include "sha-256.h"
#include "base32.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#if WINDOWS
#include <direct.h>
#include <shlobj_core.h>
#pragma comment(lib,"Shell32.lib")
#else
#include <unistd.h>
#endif

void kl_opts_new(struct kl_opts* opts, int argc, char* argv[]) {
    opts->stages = LexingStage | ParsingStage | ModAnalysisStage | TypeInferStage | TypeCheckStage | CGenStage | CCStage | ExecStage;
    opts->optim = 0;
    abl_str_new(&opts->outpath);
    opts->verbosity = 1;
    abl_vec_new(&opts->inputs, sizeof(struct kl_compiland));
    abl_vec_new(&opts->exe_argv, sizeof(char*));

    opts->color = true;

    int i = 1;
    bool endopts = false;
    while (i < argc) {
        int j = i;
        if (strcmp(argv[j], "--") == 0) {
            endopts = true;
        }
        else if (argv[j][0] == '-') {
            if (strlen(argv[j]) == 2) {
                char optopt = argv[j][1];
                char* optarg = NULL;
                if (j + 1 < argc && argv[j + 1][0] != '-') {
                    optarg = argv[j+1];
                }
                switch (optopt) {
                    case 'L':
                        opts->stages = LexingStage;
                        break;
                    case 'P':
                        opts->stages = LexingStage | ParsingStage;
                        break;
                    case 'M':
                        opts->stages = LexingStage | ParsingStage | ModAnalysisStage;
                        break;
                    case 'T':
                        opts->stages = LexingStage | ParsingStage | ModAnalysisStage | TypeInferStage | TypeCheckStage;
                        break;
                    case 'n':
                        opts->color = false;
                        break;
                    case 'v':
                        printf("Kobalt Language Compiler v%s\n\n", KLVERSION);
                        exit(0);
                        break;
                    case 'h':
                        printf("Usage: %s [file...]\n", argv[0]);
                        printf("Options:\n");
                        printf("  -o  output file\n");
                        printf("  -v  display version\n");
                        printf("  -L  lexing stage\n");
                        printf("  -P  parsing stage: produce ast\n");
                        printf("  -T  type inference stage: produce typed ast\n");
                        exit(0);
                        break;
                    case 'o':
                        {
                            i++;
                            if (optarg == NULL) {
                                abl_elog("argument to '-o' is missing");
                                exit(1);
                            }
                            abl_str_cat(&opts->outpath, optarg);
                        }
                        break;
                    default: {
                        if (isprint (optopt)) {
                            abl_elog("unknown option '-%c'", optopt);
                        }
                        else {
                            abl_elog("unknown option character '\\x%x'", optopt);
                        }
                        exit(1);
                    }
                        break;
                }
            }
            else {
                abl_elog("unknown option '%s'", argv[j]);
                exit(1);
            }
        }
        else {
            if (endopts) {
                abl_vec_push(&opts->exe_argv, &argv[j]);
            }
            else {

                struct kl_compiland compiland;
                kl_compiland_new_entry(&compiland, &argv[j][0]);
                abl_vec_push(&opts->inputs, &compiland);
            }
        }
        ++ i;
    }

    // TODO: check pipeline validation

    abl_vec_push(&opts->exe_argv, &(void*){NULL});

    size_t cwdsize = 32;
    abl_str_new(&opts->cwd);
    abl_str_resize(&opts->cwd, cwdsize);
    int maxiter = 10;
    while(maxiter-- && (getcwd(opts->cwd.data, cwdsize) == NULL)) {
        cwdsize = 2 * cwdsize;
        abl_str_resize(&opts->cwd, cwdsize);
    }
    abl_str_resize(&opts->cwd, strlen(opts->cwd.data));

    if (maxiter <= 0) {
        abl_elog("allocation");
        exit(1);
    }
    abl_path_normalize(&opts->cwd);
   
    // int seednum = kl_time_get();
    // abl_ilog("random seed is %d", seednum);
    abl_uid_seed(1337);

    abl_str_new(&opts->cachepath);
#if UNIX
    char* home = getenv("HOME");
    if(home == NULL) {
        abl_elog("cannot get HOME environment variable");
        exit(1);
    }

    abl_str_cat(&opts->cachepath, home);
    abl_path_push(&opts->cachepath, ".cache");
    abl_fs_mkdirp(opts->cachepath.data);
#else
    PWSTR appdata = NULL;
    if (SHGetKnownFolderPath(&FOLDERID_RoamingAppData, KF_FLAG_CREATE, NULL, &appdata) == S_OK) {
        char dest[MAX_PATH];
        wcstombs(dest, appdata, MAX_PATH);
        abl_str_cat(&opts->cachepath, dest);
    }
    else {
        abl_elog("getting AppData path"); 
    }
#endif
    abl_fs_mkdirp(opts->cachepath.data);
    abl_path_push(&opts->cachepath, "kobalt");
    abl_fs_mkdirp(opts->cachepath.data);
    abl_path_push(&opts->cachepath, "build");
    abl_fs_mkdirp(opts->cachepath.data);

    unsigned char data[32];
    int bsize = base32_allocated_size(32) - 1;

    calc_sha_256(data, opts->cwd.data, opts->cwd.len);

    int cachepathlen = opts->cachepath.len + 1 + bsize;

    abl_path_push(&opts->cachepath, "placeholder");
    abl_str_resize(&opts->cachepath, cachepathlen);

    base32_encode(data, 32, &opts->cachepath.data[opts->cachepath.len - bsize]);

    abl_path_normalize(&opts->cachepath);

    for(int i = 0; i < opts->cachepath.len; ++i) {
        if (opts->cachepath.data[opts->cachepath.len - 1 - i] != '=') {
            abl_str_resize(&opts->cachepath, opts->cachepath.len - i);
            break;
        }
    }

    abl_fs_mkdirp(opts->cachepath.data);

    abl_str_new(&opts->manifest_path);
    abl_str_cat(&opts->manifest_path, opts->cachepath.data);
    abl_path_push(&opts->manifest_path, "kl-build.yaml");
    opts->manifest = fopen(opts->manifest_path.data, "w");
    fprintf(opts->manifest, "workdir: %s\n", opts->cwd.data);

#if DEBUG
    // abl_ilog("cachepath is %s", opts->cachepath.data);
#endif
}

void kl_opts_del(struct kl_opts* opts) {
    abl_str_del(&opts->manifest_path);
    fclose(opts->manifest);
    abl_vec_del(&opts->inputs);
    abl_str_del(&opts->cwd);
    abl_str_del(&opts->cachepath);
    abl_str_del(&opts->outpath);
    abl_vec_del(&opts->exe_argv);
}
