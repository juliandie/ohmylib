
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <getopt.h>
#include <errno.h>
#include <pthread.h>
#include <poll.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

int i2c_open_dev(const char *i2cdev) {
    return open(i2cdev, O_RDWR);
}

int i2c_open(uint32_t i2cnum) {
    char i2cdev[32];
    snprintf(i2cdev, sizeof(i2cdev), "/dev/i2c-%u", i2cnum);
    return i2c_open_dev(i2cdev);
}

int i2c_xfer(int fd, uint16_t addr,
             uint8_t *tx, size_t tx_len,
             uint8_t *rx, size_t rx_len) {
    struct i2c_rdwr_ioctl_data msg;
    struct i2c_msg i2cmsg[2];
    unsigned int i2cmsgn = 0;

    memset(&msg, 0, sizeof(msg));
    memset(i2cmsg, 0, sizeof(i2cmsg));

    if(tx && tx_len > 0) {
        i2cmsg[i2cmsgn].addr = addr;
        i2cmsg[i2cmsgn].len = tx_len;
        i2cmsg[i2cmsgn].buf = tx;
        i2cmsgn++;
    }

    if(rx && rx_len > 0) {
        i2cmsg[i2cmsgn].addr = addr;
        i2cmsg[i2cmsgn].flags = I2C_M_RD;
        i2cmsg[i2cmsgn].len = rx_len;
        i2cmsg[i2cmsgn].buf = rx;
        i2cmsgn++;
    }

    msg.msgs = i2cmsg;
    msg.nmsgs = i2cmsgn;

    return ioctl(fd, I2C_RDWR, &msg);
}

int i2c_write(int fd, uint16_t addr, uint8_t *buf, size_t len) {
    return i2c_xfer(fd, addr, buf, len, NULL, 0);
}