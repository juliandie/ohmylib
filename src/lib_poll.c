#include <lib_poll.h>

int lib_poll(int fd, short events, int timeout_ms) {
    struct pollfd fds;

    fds.fd = fd;
    fds.events = events;
    return poll(&fds, 1, timeout_ms);
}