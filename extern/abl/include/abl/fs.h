#ifndef ABLFS__H
#define ABLFS__H

#include "abl/str.h"
#include "abl/defs.h"

ABL_API void abl_fs_mkdirp(char* path);

ABL_API void abl_fs_read2str(char* path, struct abl_str* dest);

#endif
