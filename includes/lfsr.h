#ifndef LFSR_H_
#define LFSR_H_

#include <stdint.h>
#include <stddef.h>

// GigE-LFSR; init: 0xffff; polynomial: 0x8016
size_t lfsr(uint8_t *buf, size_t count, uint16_t init, uint16_t poly);

#endif