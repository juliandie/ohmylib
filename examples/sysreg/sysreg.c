#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sysreg.h>

int main(int argc, char **argv) {
    struct sysreg_info_s i;
    uint32_t val;

    if(argc < 2) {
        fprintf(stderr, "usage: %s <value>\n", argv[0]);
    }
    else {
        val = strtoul(argv[1], NULL, 0);
        if(sysreg_write_scratch(val) < 0) {
            fprintf(stderr, "failed to write scratch-register: %s\n", strerror(errno));
            return -1;
        }
    }

    if(sysreg_read_info(&i) < 0) {
        fprintf(stderr, "failed to read info: %s\n", strerror(errno));
        return -1;
    }

    printf("%.4s\n"
           "%.4s\n"
           "%08X\n"
           "%02u:%02u:%02u\n"
           "%02u.%02u.%02u\n",
           i.project,
           i.board,
           i.version,
           i.datetime.day, i.datetime.month, i.datetime.year,
           i.datetime.hour, i.datetime.minute, i.datetime.second);

    if(sysreg_read_scratch(&val) < 0) {
        fprintf(stderr, "failed to read scratch-register: %s\n", strerror(errno));
        return -1;
    }

    printf("%08X\n", val);

    return 0;
}