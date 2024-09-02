#ifndef SX_SYSREG_H_
#define SX_SYSREG_H_

#include <stddef.h>
#include <stdint.h>

#define SYSREG_INFO_SIZE (112u)
struct sysreg_info_s {
    char project[4];
    char board[4];
    uint32_t version;
    uint32_t build;
};

int sysreg_read_info(char *buf, size_t len);

int sysreg_read_scratch(uint32_t *val);
int sysreg_write_scratch(uint32_t val);

#endif
