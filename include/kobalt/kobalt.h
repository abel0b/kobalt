#ifndef KOBALT__H
#define KOBALT__H

#include "kobalt/version.h"
#if DEBUG_SAN
#include <sanitizer/common_interface_defs.h>
#endif

#include <stdlib.h>
#include "abl/log.h"

#if WINDOWS
#define DS '\\'
#else
#define DS '/'
#endif

#ifdef WINDOWS
#define UNIX !WINDOWS
#else
#define UNIX 1
#endif

#define todo() do { fprintf(stderr, "TODO at %s:%d\n", __FILE__, __LINE__); exit(1); } while(0)

#define kl_todo(...) do { abl_elog("TODO feature at %s:%s:%d", __FILE__, __func__, __LINE__); abl_elog(__VA_ARGS__); exit(1); } while(0)

#define unused(X) do { (void)(X); } while(0)

#define kl_unused(X) do { (void)(X); } while(0)

#if DEBUG_SAN
#define kl_exit(X) do {if (X) {__sanitizer_print_stack_trace(); exit(X);}else{exit(X);}} while(0)
#else
#define kl_exit(X) exit(X)
#endif

#endif
