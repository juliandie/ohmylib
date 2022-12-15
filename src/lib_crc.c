
#include "lib_crc.h"

static uint8_t crc8_reflect(uint8_t c) {
    uint8_t ret = 0;

    for(int i = 0; i < 8; i++)
        if(c & (0x80 >> i)) 
            ret |= (0x01 << i);
    
    return ret;
}

uint8_t crc8(uint8_t v, uint8_t c, uint8_t poly, int refin, int refout) {
    uint8_t i;
    
    if(refin)
        c = crc8_reflect(c);

    for (i = 0; i < 8; i++) {
        if ((c ^ v) & 0x80u)
            c = (c << 1) ^ poly;
        else
            c <<= 1;
        v <<= 1;
    }
    
    if(refout)
        c = crc8_reflect(c);

    return c;
}

uint16_t crc16(uint16_t v, uint16_t c, uint16_t poly) {
    uint16_t i;

    for (i = 0; i < 32; i++) {
        if((c ^ v) & 0x8000u)
            c = (c << 1) ^ poly;
        else
            c = (c << 1);
    }

    return c;
}

uint32_t crc32(uint32_t v, uint32_t c, uint32_t poly) {
    uint32_t i;

    for (i = 0; i < 32; i++) {
        if((c ^ v) & 0x80000000u)
            c = (c << 1) ^ poly;
        else
            c = (c << 1);
    }

    return c;
}