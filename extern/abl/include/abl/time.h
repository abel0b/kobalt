#ifndef ABLTIME__H
#define ABLTIME__H

#include <stdint.h>
#include "abl/defs.h"

#if UNIX
#include <sys/time.h>
#else
#include <windows.h>
#endif

struct abl_timer {
    struct timeval start;
    struct timeval end;
};

ABL_API void abl_timer_start(struct abl_timer* timer);
ABL_API int abl_timer_end(struct abl_timer* timer);

ABL_API int abl_time_get();
ABL_API uint64_t abl_time_get_us();

#endif
