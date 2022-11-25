#include <lib_time.h>
#include <stdio.h>
#include <stdint.h> // uint*_t
#include <stdlib.h> // malloc, free
#include <string.h> // memset
#include <unistd.h> // usleep
#include <errno.h>
#include <time.h> // timer_*, nanosleep
#include <sys/time.h>
#include <signal.h>

int lib_timestamp_create(timer_t *tid) {
    struct sigevent ev = {
        .sigev_notify = SIGEV_NONE,
    };

    return timer_create(CLOCK_REALTIME, &ev, tid);
}

int lib_timestamp_start(timer_t tid) {
    struct itimerspec it_new = { 
        .it_interval = { .tv_sec = INT32_MAX, .tv_nsec = 0 },
        .it_value = { .tv_sec = INT32_MAX, .tv_nsec = 0 }
    };
    return timer_settime(tid, 0, &it_new, NULL);
}

int lib_timestamp_resume(timer_t tid, struct itimerspec *it_new) {
    return timer_settime(tid, 0, it_new, NULL);
}

int lib_timestamp_stop(timer_t tid, struct itimerspec *it_old) {
    struct itimerspec it_new = {};
    // timer is still running, but won't restart after expire
    return timer_settime(tid, 0, &it_new, it_old);
}

int lib_timestamp_elapsed(timer_t tid, struct itimerspec *it_elapsed) {
    ldiv_t ldiv_val;
    uint64_t nsec;

    if(timer_gettime(tid, it_elapsed) < 0)
        return -1;

    // timer_gettime should return EFAULT whenn it == NULL
    if(it_elapsed) {
        nsec = ((uint64_t)INT32_MAX * 1000000000) -
            ((it_elapsed->it_value.tv_sec * 1000000000) + 
             it_elapsed->it_value.tv_nsec);
        ldiv_val = ldiv(nsec, 1000000000);
        it_elapsed->it_value.tv_sec = ldiv_val.quot;
        it_elapsed->it_value.tv_nsec = ldiv_val.rem;
    }

    return 0;
}

void lib_timestamp_delete(timer_t tid) {
    timer_delete(tid);
}