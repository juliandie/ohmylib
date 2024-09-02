#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#include <sysreg.h>

#define SYSREG_SCRATCH  "/proc/sysreg/scratch"
#define SYSREG_INFO     "/proc/sysreg/info"
#define SYSREG_PROJECT  "/proc/sysreg/project"
#define SYSREG_BOARD    "/proc/sysreg/board"
#define SYSREG_VERSION  "/proc/sysreg/version"
#define SYSREG_TIMEDATE "/proc/sysreg/timedate"

/** /proc/sysreg/info
 * Project  : %.4s
 * Version  : %08X
 * Board    : %.4s
 * Buildreg : %08X
 * Buildtime: %02d:%02d:%02d
 * Builddate: %02d.%02d.%02d
 */

int sysreg_read_info(struct sysreg_info_s *i) {
    FILE *f;
    int ret;

    f = fopen(SYSREG_PROJECT, "r");
    if(!f) {
        return -1;
    }
    ret = fread(&i->project, 1, sizeof(i->project), f);
    if(ret < 0) {
        goto err_fclose;
    }
    fclose(f);

    f = fopen(SYSREG_BOARD, "r");
    if(!f) {
        return -1;
    }
    ret = fread(&i->board, 1, sizeof(i->board), f);
    if(ret < 0) {
        goto err_fclose;
    }
    fclose(f);

    f = fopen(SYSREG_VERSION, "r");
    if(!f) {
        return -1;
    }
    ret = fread(&i->version, 1, sizeof(i->version), f);
    if(ret < 0) {
        goto err_fclose;
    }
    fclose(f);

    f = fopen(SYSREG_TIMEDATE, "r");
    if(!f) {
        return -1;
    }
    ret = fread(&i->datetime.raw, 1, sizeof(i->datetime.raw), f);
    if(ret < 0) {
        goto err_fclose;
    }

err_fclose:
    fclose(f);
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