#ifndef LIB_TIME_H_
#define LIB_TIME_H_

#include <time.h> // timer_t

/** timer without event, expires after 5 seconds
#include <time.h>
#include <signal.h>
int main() {
    timer_t tid;
    struct sigevent ev_sig = {
        .sigev_notify = SIGEV_NONE,
    };
    // used to stop timer
    struct itimerspec stop_tim = {};
    // used to store timer values
    struct itimerspec old_tim = {};
    struct itimerspec new_tim = {
		.it_value = { .tv_sec = 5, .tv_nsec = 0, }
    };
    int ret;
    //...
    // create a timer - See man 2 timer_create
    ret = timer_create(CLOCK_REALTIME, &ev_sig, &tid);
    if(ret < 0) return -1; //... error
    //...
    // start the timer
    ret = timer_settime(tid, 0, &new_tim, NULL);
    if(ret < 0) return -1; //... error
    //...
    // check if timer has expired
    ret = timer_gettime(tid, &old_tim);
    if(ret < 0) return -1; //... error
    else if(ret == 0) {}//... not expired
    else {} // (ret > 0) //... expired
    //...
    ret = timer_settime(tid, 0, &stop_tim, &old_tim);
    if(ret < 0) return -1; //... error
    //...
    // free resources
    timer_delete(tid);
}
 */

/** timer with event, repeated every 3 seconds
#include <time.h>
#include <signal.h>
int main() {
    timer_t tid;
    struct sigevent ev_alsig = {
    // Use SIGALRM as signal when the timer has expired
        .sigev_signo = SIGALRM,
        .sigev_notify = SIGEV_SIGNAL,
    };
    // used to store timer values
    struct itimerspec old_tim = {};
    struct itimerspec new_tim = {
        // autoreload after expire
		.it_interval = { .tv_sec = 3, .tv_nsec = 0, },
		.it_value = { .tv_sec = 3, .tv_nsec = 0, }
    };
    int ret;

    // create a timer - See man 2 timer_create
    ret = timer_create(CLOCK_REALTIME, &ev_alsig, &tid);
    if(ret < 0) return -1;//... error
    //...
    // start the timer
    ret = timer_settime(tid, 0, &new_tim, NULL);
    if(ret < 0) return -1; //... error
    //...
    ret = timer_gettime(tid, &old_tim);
    if(ret < 0) return -1; //... error
    //...
    // free resources
    timer_delete(tid);
}
*/

#ifdef __cplusplus
extern "C" {
#endif // defined (__cplusplus)

int lib_timestamp_create(timer_t *tid);
int lib_timestamp_start(timer_t tid);
int lib_timestamp_resume(timer_t tid, struct itimerspec *it_new);
int lib_timestamp_stop(timer_t tid, struct itimerspec *it_old);
int lib_timestamp_elapsed(timer_t tid, struct itimerspec *it_elapsed);
void lib_timestamp_delete(timer_t tid);

#ifdef __cplusplus
}
#endif // defined (__cplusplus)

#endif