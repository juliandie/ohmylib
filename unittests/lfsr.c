#include <stdio.h>

#include <lib_log.h>
#include <lib_lfsr.h>

int main() {
    char buf[0x100];

    lib_lfsr(buf, sizeof(buf), 0xffff, 0x8016);
    lib_hexdump(buf, sizeof(buf), "lfsr");

    return 0;
}