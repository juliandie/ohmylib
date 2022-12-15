#ifndef OHMYLIB_LFSR_H_
#define OHMYLIB_LFSR_H_

#include <stdint.h>
#include <stddef.h>

// GigE-LFSR; init: 0xffff; polynomial: 0x8016
size_t lib_lfsr(uint8_t *buf, size_t count, uint16_t init, uint16_t poly);

#endif