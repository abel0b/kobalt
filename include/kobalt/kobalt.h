#ifndef KOBALT__H
#define KOBALT__H

#include "kobalt/version.h"
#if DEBUG
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#include <sanitizer/common_interface_defs.h>
#endif
#endif
#endif

#include <stdlib.h>
#include "klbase/log.h"

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

#define kl_todo(...) do { kl_elog("TODO feature at %s:%s:%d", __FILE__, __func__, __LINE__); kl_elog(__VA_ARGS__); exit(1); } while(0)

#define unused(X) do { (void)(X); } while(0)

#define kl_unused(X) do { (void)(X); } while(0)

#if DEBUG
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define exit(X) do {if (X) {__sanitizer_print_stack_trace(); exit(X);}else{exit(X);}} while(0)
#endif
#endif
#endif

#endif
