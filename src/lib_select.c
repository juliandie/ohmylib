#include <lib_select.h>

int lib_select_read(int fd, int timeout) {
    struct timeval tv;
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    return select(fd + 1, &fds, NULL, NULL, &tv);
}