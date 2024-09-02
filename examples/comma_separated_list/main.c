#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <cstring.h>

int main(int argc, char **argv) {
    char *nxt, *str;

    if(argc < 2) {
        printf("usage: %s comma,separated,list\n", argv[0]);
        return -1;
    }

    iter_substr(str, argv[1], nxt, ',') {
        printf("%s\n", str);
        free(str);
    }

    return 0;
}