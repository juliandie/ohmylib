#ifndef OHMYLIB_POLL_H_
#define OHMYLIB_POLL_H_

#include <poll.h>

int lib_poll(int fd, short events, int timeout);

#endif /* OHMYLIB_POLL_H_ */