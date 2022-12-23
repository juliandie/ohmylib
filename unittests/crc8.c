#include <stdio.h>
#include <stdint.h>

#include <lib_log.h>
#include <lib_crc.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static struct crc8_s {
    uint8_t result;
    uint8_t polynom;
    uint8_t crc; // init
    int refin;
    int refout;
} crc8s[] = {
    {0x83, 0x07, 0x00, 0, 0},
    {0xBC, 0x9B, 0xFF, 0, 0},
    {0x20, 0x39, 0x00, 1, 1},
    {0xFE, 0xD5, 0x00, 0, 0},
    {0x34, 0x1D, 0xFF, 1, 1},
    {0x10, 0x1D, 0xFD, 0, 0},
    {0xD6, 0x07, 0x00, 0, 0},
    {0x7C, 0x31, 0x00, 1, 1},
    {0x6D, 0x07, 0xFF, 1, 1},
    {0xFD, 0x9B, 0x00, 1, 1},
};
static struct crc32_s {
    uint32_t result;
    uint32_t polynom;
    uint32_t crc; // init
} crc32s[] = {
    {0x519025E9, 0x04C11DB7, 0xFFFFFFFF},
    {0xABF72CBD, 0x04C11DB7, 0xFFFFFFFF},
    {0xAE6FDA16, 0x04C11DB7, 0xFFFFFFFF},
    {0x5408D342, 0x04C11DB7, 0xFFFFFFFF},
    {0x6C6EF783, 0x04C11DB7, 0x00000000},
    {0x7F713F64, 0x04C11DB7, 0x52325032},
    {0x0493FE36, 0x000000AF, 0x00000000},
    {0x190097B3, 0x1EDC6F41, 0xFFFFFFFF},
    {0x99786C4F, 0xA833982B, 0xFFFFFFFF},
    {0xA128F1CD, 0x814141AB, 0x00000000},
};

static uint8_t my_crc8 = 0;
static uint8_t mycrc8(uint8_t v, uint8_t a) {
    uint8_t c, i;
    uint8_t poly = 0x8c;

    c = a;
    for (i = 0; i < 8; i++) {
        if ((c ^ v) & 1)
            c = (c >> 1) ^ poly;
        else
            c >>= 1;
        v >>= 1;
    }
    return c;
}


// copied from some 1-wire application
static void do_crc8(const char v) {
    struct crc8_s *c;
    for(size_t i = 0; i < ARRAY_SIZE(crc8s); i++) {
        c = &crc8s[i];
        c->crc = crc8(v, c->crc, c->polynom, c->refin, c->refout);
    }
    my_crc8 = mycrc8(v, my_crc8);
}

static void do_crc32(const char v) {
    struct crc32_s *c;
    for(size_t i = 0; i < ARRAY_SIZE(crc8s); i++) {
        c = &crc32s[i];
        c->crc = crc32(v, c->crc, c->polynom);
    }
}

static void crc_print() {
    struct crc8_s *c8;
    struct crc32_s *c32;
    printf("CRC        Res        Poly       RefIn   RefOut\n");
    for(size_t i = 0; i < ARRAY_SIZE(crc8s); i++) {
        c8 = &crc8s[i];
        printf("0x%08x    0x%08x    0x%08x    0x%02x    0x%02x\n",
               c8->crc, c8->result, c8->polynom, c8->refin, c8->refout);
    }
    for(size_t i = 0; i < ARRAY_SIZE(crc32s); i++) {
        c32 = &crc32s[i];
        printf("0x%08x    0x%08x    0x%08x\n",
               c32->crc, c32->result, c32->polynom);
    }
    printf("0x%08x    0x%08x    0x%08x    0x%02x    0x%02x\n",
            my_crc8, 0, 0x8c, 0, 0);
}

int main(int argc, char **argv) {
    char buf;
    FILE *fp;

    if(argc < 2) {
        printf("usage: %s <filename/string>\n", argv[0]);
        return -1;
    }

    fp = fopen(argv[1], "r");
    if(!fp) {
        //perror("fopen");
        for(size_t i = 0; i < strlen(argv[1]); i++) {
            do_crc8(argv[1][i]);
            do_crc32(argv[1][i]);
        }
    }
    else {
        while(fread(&buf, 1, 1, fp) > 0) {
            do_crc8(buf);
            do_crc32(buf);
        }

        fclose(fp);
    }

    crc_print();

    return 0;
}