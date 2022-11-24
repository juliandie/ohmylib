
#include "lib_crc8.h"

#define CRC8_POLY 0x8C

uint8_t crc8(uint8_t v, uint8_t a) {
    uint8_t c, i;

    c = a;
    for (i = 0; i < 8; i++) {
        if ((c ^ v) & 1)
            c = (c >> 1) ^ CRC8_POLY;
        else
            c >>= 1;
        v >>= 1;
    }
    return c;
}
