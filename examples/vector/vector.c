#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <cvector.h>

struct some_struct_s {
    int val;
};

int main() {
    struct some_struct_s **v = NULL, **tmp = NULL;
    struct some_struct_s *s;

    for(int i = 0; i < 0x100; i++) {
        s = calloc(1, sizeof(*s));
        if(!s) {
            fprintf(stderr, "failed to calloc: %s\n", strerror(errno));
            freev(v);
            exit(EXIT_FAILURE);
        }
        s->val = i + 1;

        tmp = pushv(s, v);
        if(!tmp) {
            fprintf(stderr, "failed to pushv: %s\n", strerror(errno));
            free(s);
            freev(v);
            exit(EXIT_FAILURE);
        }
        v = tmp;
    }

    printf("vectors has %lu elements\n", countv(v));
    int flipflop = 0;
    do {
        /** switch between poping from back and front */
        if(flipflop) {
            s = popv(v);
            if(!s) {
                continue;
            }
            printf("%d; ", s->val);
            free(s);
        }
        else {
            s = popv_front(v);
            if(!s) {
                continue;
            }
            printf("%d; ", s->val);
            free(s);
        }
        flipflop = 1 - flipflop;
    } while(s != NULL);

    freev(v);
    return 0;
}