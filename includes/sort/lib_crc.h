#ifndef OHMYLIB_CRC8_H_
#define OHMYLIB_CRC8_H_

#include <stdint.h>

/*
 * Reference: "The quick brown fox jumps over the lazy dog."
 * Algorithm	        Result	    Poly	    Init	    RefIn	RefOut	XorOut
 * CRC-8                0x83	    0x07	    0x00	    false	false	0x00
 * CRC-8/CDMA2000       0xBC	    0x9B	    0xFF	    false	false	0x00
 * CRC-8/DARC           0x20	    0x39	    0x00	    true	true	0x00
 * CRC-8/DVB-S2         0xFE	    0xD5	    0x00	    false	false	0x00
 * CRC-8/EBU            0x34	    0x1D	    0xFF	    true	true	0x00
 * CRC-8/I-CODE         0x10	    0x1D	    0xFD	    false	false	0x00
 * CRC-8/ITU            0xD6	    0x07	    0x00	    false	false	0x55
 * CRC-8/MAXIM          0x7C	    0x31	    0x00	    true	true	0x00
 * CRC-8/ROHC           0x6D	    0x07	    0xFF	    true	true	0x00
 * CRC-8/WCDMA          0xFD	    0x9B	    0x00	    true	true	0x00
 * CRC-16/ARC           0x843D	    0x8005	    0x0000	    true	true	0x0000
 * CRC-16/AUG-CCITT     0xEA0C	    0x1021	    0x1D0F	    false	false	0x0000
 * CRC-16/BUYPASS       0xAFA4	    0x8005	    0x0000	    false	false	0x0000
 * CRC-16/CCITT-FALSE   0x78CB	    0x1021	    0xFFFF	    false	false	0x0000
 * CRC-16/CDMA2000      0xC0A4	    0xC867	    0xFFFF	    false	false	0x0000
 * CRC-16/DDS-110       0x5C8C	    0x8005	    0x800D	    false	false	0x0000
 * CRC-16/DECT-R        0x44B2	    0x0589	    0x0000	    false	false	0x0001
 * CRC-16/DECT-X        0x44B3	    0x0589	    0x0000	    false	false	0x0000
 * CRC-16/DNP           0x60DA	    0x3D65	    0x0000	    true	true	0xFFFF
 * CRC-16/EN-13757      0x6F8A	    0x3D65	    0x0000	    false	false	0xFFFF
 * CRC-16/GENIBUS       0x8734	    0x1021	    0xFFFF	    false	false	0xFFFF
 * CRC-16/KERMIT        0x07FC	    0x1021	    0x0000	    true	true	0x0000
 * CRC-16/MAXIM         0x7BC2	    0x8005	    0x0000	    true	true	0xFFFF
 * CRC-16/MCRF4XX       0x19A5	    0x1021	    0xFFFF	    true	true	0x0000
 * CRC-16/MODBUS        0x7528	    0x8005	    0xFFFF	    true	true	0x0000
 * CRC-16/RIELLO        0x9340	    0x1021	    0xB2AA	    true	true	0x0000
 * CRC-16/T10-DIF       0x3B66	    0x8BB7	    0x0000	    false	false	0x0000
 * CRC-16/TELEDISK      0xCB11	    0xA097	    0x0000	    false	false	0x0000
 * CRC-16/TMS37157      0xAF97	    0x1021	    0x89EC	    true	true	0x0000
 * CRC-16/USB           0x8AD7	    0x8005	    0xFFFF	    true	true	0xFFFF
 * CRC-16/X-25          0xE65A	    0x1021	    0xFFFF	    true	true	0xFFFF
 * CRC-16/XMODEM        0xE2B3	    0x1021	    0x0000	    false	false	0x0000
 * CRC-A                0x159F	    0x1021	    0xC6C6	    true	true	0x0000
 * CRC-32               0x519025E9	0x04C11DB7	0xFFFFFFFF	true	true	0xFFFFFFFF
 * CRC-32/BZIP2         0xABF72CBD	0x04C11DB7	0xFFFFFFFF	false	false	0xFFFFFFFF
 * CRC-32/JAMCRC        0xAE6FDA16	0x04C11DB7	0xFFFFFFFF	true	true	0x00000000
 * CRC-32/MPEG-2        0x5408D342	0x04C11DB7	0xFFFFFFFF	false	false	0x00000000
 * CRC-32/POSIX         0x6C6EF783	0x04C11DB7	0x00000000	false	false	0xFFFFFFFF
 * CRC-32/SATA          0x7F713F64	0x04C11DB7	0x52325032	false	false	0x00000000
 * CRC-32/XFER          0x0493FE36	0x000000AF	0x00000000	false	false	0x00000000
 * CRC-32C              0x190097B3	0x1EDC6F41	0xFFFFFFFF	true	true	0xFFFFFFFF
 * CRC-32D              0x99786C4F	0xA833982B	0xFFFFFFFF	true	true	0xFFFFFFFF
 * CRC-32Q              0xA128F1CD	0x814141AB	0x00000000	false	false	0x00000000
 */

uint8_t crc8(uint8_t v, uint8_t c, uint8_t poly, int refin, int refout);
uint16_t crc16(uint16_t v, uint16_t c, uint16_t poly);
uint32_t crc32(uint32_t v, uint32_t c, uint32_t poly);

#endif
