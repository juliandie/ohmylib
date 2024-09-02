#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <gpiod.h>

int main(int argc, char **argv) {
    struct gpiod_chip *chip;
    char devname[32];

    if(argc < 3) {
        fprintf(stderr, "usage: %s <gpiochip> <gpioline>[=<value>] ...\n", argv[0]);
        return -1;
    }

    snprintf(devname, sizeof(devname), "/dev/gpiochip%u", (uint32_t)atoi(argv[1]));
    chip = gpiod_chip_open(devname);
    if(!chip) {
        fprintf(stderr, "failed to open gpiochip %s: %s\n", devname, strerror(errno));
        return -1;
    }

    for(int i = 2; i < argc; i++) {
        struct gpiod_line *line;
        char *str, *linenum, *value;

        str = strdup(argv[i]);
        if(!str) {
            fprintf(stderr, "failed to duplicate argv[%d]: %s\n", i, strerror(errno));
            goto err_gpiod_chip_close;
        }

        linenum = str;
        value = strchr(str, '=');
        if(value) {
            value[0] = '\0';
            value++;
        }

        line = gpiod_chip_get_line(chip, atoi(linenum));
        if(value) {
            if(gpiod_line_request_output(line, "gpo", 0) < 0) {
                fprintf(stderr, "failed to request output: %s\n", strerror(errno));
                gpiod_line_release(line);
                goto err_gpiod_chip_close;
            }

            if(gpiod_line_set_value(line, atoi(value)) < 0) {
                fprintf(stderr, "failed to set output: %s\n", strerror(errno));
                gpiod_line_release(line);
                goto err_gpiod_chip_close;
            }
        }
        else {
            if(gpiod_line_request_input(line, "gpi") < 0) {
                fprintf(stderr, "failed to request input: %s\n", strerror(errno));
                gpiod_line_release(line);
                goto err_gpiod_chip_close;
            }

            printf("%d %d\n", atoi(linenum), gpiod_line_get_value(line));
        }

        gpiod_line_release(line);
        free(str);
    }

    return 0;

err_gpiod_chip_close:
    gpiod_chip_close(chip);
    return -1;
}