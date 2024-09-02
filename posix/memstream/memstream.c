#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <printlog.h>

int main(int argc, char **argv) {
    char buf[9];
    char *ptr = NULL;
    size_t sizeloc, len;
    FILE *f;
    int wc;

    if(argc < 2) {
        wc = 1;
    }
    else {
        wc = atoi(argv[1]);
        if(wc < 1) {
            wc = 1;
        }
    }

    memset(buf, 0x37, sizeof(buf) - 1);
    printf("strlen(buf): %lu\n", strlen(buf));
    f = open_memstream(&ptr, &sizeloc);
    if(!f) {
        perror("open_memstream");
        return -1;
    }

    for(int i = 0; i < wc; i++) {
        len = fwrite(buf, 1, strlen(buf), f);
        if(len != strlen(buf)) {
            perror("fwrite");
            goto err;
        }
        fflush(f); // fwrite updates ptr and sizeloc on fflush and fclose
    }
    
    fclose(f); // fwrite updates ptr and sizeloc on fflush and fclose
    hexdump(ptr, sizeloc, "memstream buffer");
    free(ptr);
    return 0;

err:
    fclose(f);
    free(ptr);
    return -1;
}