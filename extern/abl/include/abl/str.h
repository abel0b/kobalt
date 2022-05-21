#ifndef ABLSTR__H
#define ABLSTR__H

#include "abl/vec.h"
#include "abl/defs.h"

struct abl_str {
    char* data;
    int len;
    int cap;
};

ABL_API void abl_str_new(struct abl_str* str);
ABL_API void abl_str_catf(struct abl_str* str, char* fmt, ...);
ABL_API void abl_str_cat(struct abl_str* str, char* src);
ABL_API void abl_str_resize(struct abl_str* str, long int len);
ABL_API void abl_str_del(struct abl_str* str);

abl_vec_decl(struct abl_str, str)

#endif
