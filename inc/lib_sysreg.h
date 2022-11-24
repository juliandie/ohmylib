#ifndef OHMYLIB_SYSREG_H_
#define OHMYLIB_SYSREG_H_

#include <stdint.h>
#include <stddef.h>

#define SYSREG_INFO_SIZE (112u)

int sysreg_read_info(char *buf, size_t len);

int sysreg_read_scratch(uint32_t *val);
int sysreg_write_scratch(uint32_t val);

#endif
