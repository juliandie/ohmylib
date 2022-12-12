#include <lib_lfsr.h>

size_t lib_lfsr(uint8_t *buf, size_t count, uint16_t init, uint16_t poly) {
    uint16_t lfsr = init;
    uint8_t *p;

    if(!buf)
        return 0;

    for(p = buf; p < buf + count; p++) {
        *p = (lfsr & 0xFF);
        lfsr = ((lfsr >> 1) ^ (-(lfsr & 1) & poly));
    }

    return count;
}