#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

static int i2c_open_dev(const char *i2cdev) {
    return open(i2cdev, O_RDWR);
}

static int i2c_open(uint32_t i2cnum) {
    char i2cdev[20];
    snprintf(i2cdev, sizeof(i2cdev), "/dev/i2c-%u", i2cnum);
    return i2c_open_dev(i2cdev);
}

static int i2c_xfer(int fd, uint16_t addr,
                    uint8_t *tx, size_t tx_len,
                    uint8_t *rx, size_t rx_len) {
    struct i2c_rdwr_ioctl_data msgs;
    struct i2c_msg msg[2];

    memset(&msgs, 0, sizeof(msgs));
    memset(msg, 0, sizeof(msg));
    msgs.msgs = msg;

    if(tx && tx_len > 0) {
        msg[msgs.nmsgs].addr = addr;
        msg[msgs.nmsgs].flags = 0;
        msg[msgs.nmsgs].len = tx_len;
        msg[msgs.nmsgs].buf = tx;
        msgs.nmsgs++;
    }

    if(rx && rx_len > 0) {
        msg[msgs.nmsgs].addr = addr;
        msg[msgs.nmsgs].flags = I2C_M_RD;
        msg[msgs.nmsgs].len = rx_len;
        msg[msgs.nmsgs].buf = rx;
        msgs.nmsgs++;
    }

    return ioctl(fd, I2C_RDWR, &msgs);
}

static int i2c_write(int fd, uint16_t addr, uint8_t *buf, size_t len) {
    return i2c_xfer(fd, addr, buf, len, NULL, 0);
}

static int i2c_write_reg8(int fd, uint16_t addr, uint8_t reg, uint8_t *buf, size_t len) {
    struct i2c_rdwr_ioctl_data msgs;
    struct i2c_msg msg[2];

    memset(&msgs, 0, sizeof(msgs));
    memset(msg, 0, sizeof(msg));
    msgs.msgs = msg;

    msg[msgs.nmsgs].addr = addr;
    msg[msgs.nmsgs].flags = 0;
    msg[msgs.nmsgs].len = sizeof(reg);
    msg[msgs.nmsgs].buf = &reg;
    msgs.nmsgs++;

    msg[msgs.nmsgs].addr = addr;
    msg[msgs.nmsgs].flags = 0;
    msg[msgs.nmsgs].len = len;
    msg[msgs.nmsgs].buf = buf;
    msgs.nmsgs++;

    return ioctl(fd, I2C_RDWR, &msgs);
}

static int i2c_write_reg16(int fd, uint16_t addr, uint16_t reg, uint8_t *buf, size_t len) {
    struct i2c_rdwr_ioctl_data msgs;
    struct i2c_msg msg[2];

    memset(&msgs, 0, sizeof(msgs));
    memset(msg, 0, sizeof(msg));
    msgs.msgs = msg;

    msg[msgs.nmsgs].addr = addr;
    msg[msgs.nmsgs].flags = 0;
    msg[msgs.nmsgs].len = sizeof(reg);
    msg[msgs.nmsgs].buf = (uint8_t *)&reg;
    msgs.nmsgs++;

    msg[msgs.nmsgs].addr = addr;
    msg[msgs.nmsgs].flags = 0;
    msg[msgs.nmsgs].len = len;
    msg[msgs.nmsgs].buf = buf;
    msgs.nmsgs++;

    return ioctl(fd, I2C_RDWR, &msgs);
}

int main(int argc, char **argv) {
    int fd;

    if(argc < 3) { 
        fprintf(stderr, "usage: %s <i2cdev> <addr> <offset>\n", argv[0]);
        return -1;
    }

    fd = i2c_open(atoi(argv[1]));
    if(fd < 0) {
        fprintf(stderr, "failed to open i2cdev: %s\n", strerror(errno));
        return -1;
    }


    close(fd);
    return 0;
}