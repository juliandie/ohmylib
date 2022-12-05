#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <lib_log.h>

const char str[] = "test";

int main(int argc, char **argv) {
    char *ptr = NULL;
    size_t sizeloc, len;
    FILE *f;

    (void)argc;
    (void)argv;

    f = open_memstream(&ptr, &sizeloc);
    if(!f) {
        perror("open_memstream");
        return -1;
    }

    len = fwrite(str, 1, strlen(str), f);
    if(len != strlen(str)) {
        perror("fwrite");
        goto err;
    }

    fflush(f); // fwrite is written on fflush and fclose
    lib_hexdump(ptr, sizeloc, "memstream buffer");

    fclose(f);
    free(ptr);
    return 0;

err:
    fclose(f);
    free(ptr);
    return -1;
}