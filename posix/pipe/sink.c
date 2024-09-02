#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <stdint.h>

#define BUF_SZ (8)

int main(int argc, char **argv) {
    (void)argc;
    char buf[BUF_SZ];
    int ret, count = 0;

    printf("%s\n"
           "press C^d to send EOF\n",
           argv[0]);

    for(;;) {
        memset(buf, 0, sizeof(buf));
        ret = fread(buf, 1, BUF_SZ - 1, stdin);
        if(ret <= 0) {
            if(feof(stdin)) {
                fprintf(stderr, "end of file\n");
                break;
            }
            if(ferror(stdin)) {
                fprintf(stderr, "fread error\n");
                break;
            }
            fprintf(stderr, "unknown issue ? \n");
            break;
        }
        fprintf(stdout, "received %s\n", buf);
        count++;
    }

    printf("read %d * %d\n", count, BUF_SZ - 1);

    return 0;
}