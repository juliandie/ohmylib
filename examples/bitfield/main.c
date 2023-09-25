#include <stdio.h>
#include <stdint.h>

struct test {
    uint8_t a : 1;
    uint8_t b : 2;
    uint8_t c : 1;
    uint8_t res : 4;
    uint16_t dunno;
};

struct yuyv {
    union {
        uint32_t raw;
        struct {
            uint8_t y0;
            uint8_t v;
            uint8_t y1;
            uint8_t u;
        };
    };
};

static struct test t;

int main() {
    struct yuyv y;
    y.y0 = 1;
    y.v = 2;
    y.y1 = 3;
    y.u = 4;
    printf("%08X\n", y.raw);
#if 0
    if(t.a)
        printf("t.s is set\n");

    t.a = 1;
    
    if(t.a)
        printf("t.s is set\n");
#endif

    return 0;
}