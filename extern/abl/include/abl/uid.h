#ifndef ABLUID__H
#define ABLUID__H

#include <stdint.h>
#include "abl/defs.h"

enum abl_uid_kind {
    ABL_UID_LALPHANUM = 0,
    ABL_UID_UALPHANUM = 1,
};

ABL_API void abl_uid_seed(uint64_t s);

ABL_API void abl_uid_gen(char uid[8], enum abl_uid_kind kind);

#endif
