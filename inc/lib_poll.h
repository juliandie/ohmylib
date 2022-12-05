#ifndef LIB_POLL_H_
#define LIB_POLL_H_

#include <poll.h>

int lib_poll(int fd, short events, int timeout);

#endif