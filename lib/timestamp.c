#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include <timestamp.h>

#define TIME_NS_IN_S (1000000000u)

int timestamp_create(timer_t *tid) {
    struct sigevent ev = {
        .sigev_notify = SIGEV_NONE,
    };

    if(timer_create(CLOCK_REALTIME, &ev, tid) < 0) {
        return -1;
    }

    return timestamp_reset(*tid);
}

void timestamp_destory(timer_t tid) {
    timer_delete(tid);
}

int timestamp_reset(timer_t tid) {
    struct itimerspec new_value = {
        .it_interval = {.tv_sec = INT32_MAX, .tv_nsec = 0 },
        .it_value = {.tv_sec = INT32_MAX, .tv_nsec = 0 }
    };
    return timer_settime(tid, 0, &new_value, NULL);
}

int timestamp_elapsed(timer_t tid, struct itimerspec *curr_value) {
    if(timer_gettime(tid, curr_value) < 0) {
        return -1;
    }

    /** timer_gettime returns EFAULT when curr_value == NULL */
    if(curr_value && curr_value->it_interval.tv_sec > 0) {
        ldiv_t ldiv_val;
        uint64_t nsec;

        nsec = (curr_value->it_interval.tv_sec * TIME_NS_IN_S) -
            ((curr_value->it_value.tv_sec * TIME_NS_IN_S) +
             curr_value->it_value.tv_nsec);
        ldiv_val = ldiv(nsec, TIME_NS_IN_S);
        curr_value->it_value.tv_sec = ldiv_val.quot;
        curr_value->it_value.tv_nsec = ldiv_val.rem;
    }

    return 0;
}