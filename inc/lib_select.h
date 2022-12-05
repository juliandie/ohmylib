#ifndef LIB_SELECT_H_
#define LIB_SELECT_H_

#include <sys/select.h>

int lib_select_read(int fd, int timeout);

#endif