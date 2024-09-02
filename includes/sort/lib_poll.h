#ifndef OHMYLIB_POLL_H_
#define OHMYLIB_POLL_H_

#include <poll.h>

static inline int lib_poll(int fd, short events, int timeout) {
    struct pollfd fds;
    fds.fd = fd;
    fds.events = events;
    return poll(&fds, 1, timeout);
}

#endif /* OHMYLIB_POLL_H_ */