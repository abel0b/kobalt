#ifndef KLTIME__H
#define KLTIME__H

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

struct kl_timer {
    struct timeval start;
    struct timeval end;
};

void kl_timer_start(struct kl_timer* timer);
int kl_timer_end(struct kl_timer* timer);

int kl_time_get();
uint64_t kl_time_get_us();

#endif
