#ifndef OHMYLIB_I2C_H_
#define OHMYLIB_I2C_H_

#include <stdint.h>
#include <stdio.h>

int i2c_open_dev(const char *i2cdev);
int i2c_open(uint32_t i2cnum);
int i2c_xfer(int fd, uint16_t addr,
             uint8_t *tx, size_t tx_len,
             uint8_t *rx, size_t rx_len);
int i2c_write(int fd, uint16_t addr, uint8_t *buf, size_t len);

#endif