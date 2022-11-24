#ifndef LIB_TIME_H_
#define LIB_TIME_H_

#include <stdint.h> // uint*_t
#include <time.h> // timer_t

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