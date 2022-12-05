#include <lib_sysreg.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#define SYSREG_INFO "/proc/sysreg/info"
#define SYSREG_SCRATCH "/proc/sysreg/scratch"

/** /proc/sysreg/info
 * Project  : %.4s
 * Version  : %08X
 * Board    : %.4s
 * Buildreg : %08X
 * Buildtime: %02d:%02d:%02d
 * Builddate: %02d.%02d.%02d
 */

int sysreg_read_info(char *buf, size_t len) {
    int ret, fd;

    fd = open(SYSREG_INFO, O_RDWR);
    if(fd < 0)
        return fd;

    ret = read(fd, buf, len);
    close(fd);

    return ret;
}

int sysreg_read_scratch(uint32_t *val) {
    uint32_t retval;
    int ret, fd;

    fd = open(SYSREG_SCRATCH, O_RDWR);
    if(fd < 0)
        return fd;

    ret = read(fd, &retval, sizeof(retval));
    close(fd);
    if(ret < 0)
        return ret;

    if(val)
        *val = retval;

    return 0;
}

int sysreg_write_scratch(uint32_t val) {
    int ret, fd;

    fd = open(SYSREG_SCRATCH, O_RDWR);
    if(fd < 0)
        return fd;

    ret = write(fd, &val, sizeof(val));
    close(fd);

    return ret;
}