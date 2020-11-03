#ifndef KBTIME__H
#define KBTIME__H

#include <kobalt/kobalt.h>
#include <stdint.h>

#if UNIX
#include <sys/time.h>
#endif

#if WINDOWS
#include <windows.h>

struct timezone;

int gettimeofday(struct timeval* tp, struct timezone* tzp);
#endif

struct kbtimer {
    struct timeval start;
    struct timeval end;
};

void kbtimer_start(struct kbtimer* timer);
int kbtimer_end(struct kbtimer* timer);

int kbtime_get();
uint64_t kbtime_get_us();

#endif
