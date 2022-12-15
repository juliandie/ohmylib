#ifndef OHMYLIB_SELECT_H_
#define OHMYLIB_SELECT_H_

#include <sys/select.h>

int lib_select_read(int fd, int timeout_sec, int timeout_usec);

#endif