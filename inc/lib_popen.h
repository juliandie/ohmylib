
#ifndef OHMYLIB_POPEN_H_
#define OHMYLIB_POPEN_H_

#include <stdint.h>
#include <unistd.h>

int lib_popen_exec(const char *cmd);
int lib_popen_read(const char *cmd, char *buf, size_t size);

#endif
