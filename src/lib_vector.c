#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <lib_vector.h>

void *lib_vector_create(size_t len, size_t num) {
}

int lib_vector_append(void **p, void *v, size_t len, size_t num) {
    void *tmp;

    tmp = realloc(*p, len * (num + 2));

    if(!tmp)
        return -1;
    
    (tmp + (len * (num))) = desc;
    (tmp + (len * (num + 1))) = desc;
    *p = tmp;

    return 0;
}