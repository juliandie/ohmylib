#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <stdint.h>
#include <stdlib.h>

#define SEND_CNT (0x100)

int main(int argc, char **argv) {
    (void)argc;
    char *buf;

    fprintf(stderr, "%s\n", argv[0]);

    buf = malloc(SEND_CNT);
    if(!buf)
        return -1;

    for(size_t i = 0; i < SEND_CNT; i++) 
        buf[i] = i & 0xff;

    fwrite(buf, 1, SEND_CNT, stdout);
    free(buf);

    return 0;
}