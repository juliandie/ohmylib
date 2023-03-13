#include <lib_select.h>
#include <stdio.h>

int lib_select_read(int fd, int tsec, int tusec) {
    struct timeval tv;
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    tv.tv_sec = tsec;
    tv.tv_usec = tusec;

    return select(fd + 1, &fds, NULL, NULL, &tv);
}