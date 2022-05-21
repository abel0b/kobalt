#ifndef ABLPROC__H
#define ABLPROC__H

#include <stdio.h>
#include "abl/defs.h"

ABL_API int abl_spawn(char* prog, char* argv[], FILE* stdoutlog);

#endif
