#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include <cvector.h>

void freev(void *ptrs) {
    if(ptrs == NULL) {
        return;
    }

    for(char **ptr = (char **)ptrs; *ptr != NULL; ptr++) {
        free(*ptr);
    }

    free(ptrs);
}

void *pushv(void *ptr, void *ptrs) {
    char **tmp = NULL;
    size_t nptrs = 2;

    if(!ptr) {
        errno = EINVAL;
        return NULL;
    }

    if(ptrs) {
        for(char **p = (char **)ptrs; *p != NULL; p++) {
            nptrs++;
        }
    }

    tmp = (char **)realloc(ptrs, nptrs * sizeof(ptr));
    if(!tmp) {
        return NULL;
    }

    tmp[nptrs - 2] = (char *)ptr;
    tmp[nptrs - 1] = NULL;
    return (void *)tmp;
}

void *popv(void *ptrs) {
    char **tmp = (char **)ptrs;
    void *ptr = NULL;
    size_t nptrs = 0;

    for(char **p = (char **)ptrs; *p != NULL; p++) {
        nptrs++;
    }

    if(nptrs == 0) {
        return NULL;
    }

    ptr = tmp[nptrs - 1];
    tmp[nptrs - 1] = NULL;
    return ptr;
}

void *popv_front(void *ptrs) {
    char **tmp = (char **)ptrs;
    void *ptr = NULL;

    ptr = tmp[0];
    for(char **p = (char **)ptrs; *p != NULL; p++) {
        p[0] = p[1];
    }

    return ptr;
}

size_t countv(void *ptrs) {
    size_t nptrs = 0;

    for(char **p = (char **)ptrs; *p != NULL; p++) {
        nptrs++;
    }

    return nptrs;
}