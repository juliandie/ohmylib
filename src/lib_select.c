#include <lib_select.h>
#include <stdio.h>

int lib_select_read(int fd, int timeout_sec, int timeout_usec) {
    struct timeval tv;
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    tv.tv_sec = timeout_sec;
    tv.tv_usec = timeout_usec;

    return select(fd + 1, &fds, NULL, NULL, &tv);
}