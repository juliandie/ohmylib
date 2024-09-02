#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

int timestamp_create(timer_t *tid);
void timestamp_destory(timer_t tid);
int timestamp_reset(timer_t tid);
int timestamp_elapsed(timer_t tid, struct itimerspec *curr_value);

#ifdef __cplusplus
}
#endif

#endif