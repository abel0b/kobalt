#ifndef ABLPATH__H
#define ABLPATH__H

#include "abl/str.h"

ABL_API void abl_path_push(struct abl_str* path, char* subpath);

ABL_API void abl_path_pushf(struct abl_str* path, char* fmt, ...);

ABL_API void abl_path_normalize(struct abl_str* path);

ABL_API char* abl_path_basename(struct abl_str* path);

ABL_API int abl_path_isds(char c);

#endif
