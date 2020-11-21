#ifndef KOBALT__H
#define KOBALT__H

#include "kobalt/version.h"

#include <stdlib.h>
#include "kobalt/log.h"

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

#define kb_todo(...) do { kbelog("TODO feature at %s:%s:%d", __FILE__, __func__, __LINE__); kbelog(__VA_ARGS__); exit(1); } while(0)

#define unused(X) do { (void)(X); } while(0)

#define kb_unused(X) do { (void)(X); } while(0)

#endif
