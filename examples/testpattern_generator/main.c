#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/videodev2.h>

static int testpattern_store(size_t count) {
    char fn[256];
    int fd, ret;
    size_t size = 0;

    if(snprintf(fn, sizeof(fn), "testpattern.bin") < 0)
        goto err;

    fd = open(fn, O_RDWR | O_CREAT);
    if(fd < 0)
        goto err;

    while(size < count) {
        ret = write(fd, (uint8_t)(size & 0xff), 1);
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
    int ret;

    if(argc < 1) {
    usage(argv[0]);
        return -1;
    }

    return testpattern_store(atol(argv[1]));
}