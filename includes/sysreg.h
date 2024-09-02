#ifndef SYSREG_H_
#define SYSREG_H_

/**
 * Solectrix sysreg is an AXI-Bus module that provides some short
 * information about the FPGA-Module. See struct sysreg_info_s.
 */

#include <stddef.h>
#include <stdint.h>

#define SYSREG_INFO_SZ (112u)

/** struct sysreg_datetime_s -
 * @day:
 * @month:
 * @year:
 * @hour:
 * @minute:
 * @second:
 * @raw:    
 */
struct sysreg_datetime_s {
    union {
        struct {
            uint32_t day : 5;
            uint32_t month : 4;
            uint32_t year : 6;
            uint32_t hour : 5;
            uint32_t minute : 6;
            uint32_t second : 6;
        };
        uint32_t raw;
    };
};

/** struct sysreg_info_s -
 * @project: short name of the project
 * @board:   short name of the hardware
 * @version: fpga version
 * @build:   fpga build
 */
struct sysreg_info_s {
    char project[4];
    char board[4];
    uint32_t version;
    struct sysreg_datetime_s datetime;
};

int sysreg_read_info(struct sysreg_info_s *i);

int sysreg_read_scratch(uint32_t *val);
int sysreg_write_scratch(uint32_t val);

#endif