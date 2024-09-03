#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <module.h>

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    do_initcalls();
    printf("do something...\n");
    do_exitcalls();
    return 0;
}

