#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

/** Features
 * Add GVSP testpattern
 * Add incremental testpattern
 * Add sawtooth testpattern
 * Add bar testpattern
 * Add output filename
 */

static int testpattern_store(size_t count) {
    char fn[256];
    int fd, ret;
    size_t size = 0;
    uint8_t val;

    if(snprintf(fn, sizeof(fn), "testpattern.bin") < 0)
        goto err;

    fd = open(fn, O_RDWR | O_CREAT);
    if(fd < 0)
        goto err;

    while(size < count) {
        val = (uint8_t)(size & 0xff);
        ret = write(fd, &val, 1);
        if(ret < 0)
            goto err_close;

        size += (size_t)ret;
    }

    close(fd);

    return 0;

err_close:
    close(fd);
err:
    return -1;
}

static void usage(const char *argv0) {
    printf("usage: %s <size>\n", argv0);
}

int main(int argc, char **argv) {
    if(argc < 1) {
    usage(argv[0]);
        return -1;
    }

    return testpattern_store(strtoul(argv[1], NULL, 0));
}
