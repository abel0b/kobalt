#ifndef KOBALT__H
#define KOBALT__H

#define KBVERSION "0.1.0"

#include <stdlib.h>

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

#define unused(X) do { (void)(X); } while(0)

#endif
