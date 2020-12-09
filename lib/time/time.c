#include "klbase/time.h"
#include "klbase/log.h"

#if WINDOWS
// credit: https://stackoverflow.com/a/26085827
int gettimeofday(struct timeval * tp, struct timezone * tzp) {
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970 
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#endif

void kl_timer_start(struct kl_timer* timer) {
    int rc = gettimeofday(&timer->start, NULL);
    if (rc != 0) exit(1);
}

int kl_timer_end(struct kl_timer* timer) { 
    int rc = gettimeofday(&timer->end, NULL);
    if (rc != 0) exit(1);
    int time_us = (timer->end.tv_sec - timer->start.tv_sec) * 1000000 + (timer->end.tv_usec - timer->start.tv_usec);
    return time_us;
}

int kl_time_get() {
    struct timeval tv;
    int rc = gettimeofday(&tv, NULL);
    if (rc != 0) {
        kl_elog("could not get time");
        exit(1);
    }
    return tv.tv_sec;
}

uint64_t kl_time_get_us() {
    struct timeval tv;
    int rc = gettimeofday(&tv, NULL);
    if (rc != 0) {
        kl_elog("could not get time");
        exit(1);
    }
    return tv.tv_sec * 1000000 + tv.tv_usec;
}
