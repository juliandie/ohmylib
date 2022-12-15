
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#include <lib_popen.h>

int lib_popen_exec(const char *cmd) {
    FILE *fp;

    fp = popen(cmd, "r");
    if(!fp)
        return -1;

    return WEXITSTATUS(pclose(fp));
}

int lib_popen_read(const char *cmd, char *buf, size_t size) {
    FILE *fp;

    fp = popen(cmd, "r");
    if(!fp)
        return -1;

    if(!fgets(buf, size, fp))
        goto err;

    return WEXITSTATUS(pclose(fp));

err:
    pclose(fp);
    return -1;
}