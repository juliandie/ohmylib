#include <stdio.h>
#include <stdlib.h>

#include <lib_log.h>

char *arr[] = {
    "12345",
    "asdf",
    "abcdefghijklmno",
    NULL
};

/**
 * The vector is always NULL terminated.
 */
static int addv(char ***v, char *p, size_t nmemb) {
    char **tmp;

    if(!nmemb)
        tmp = malloc(2 * sizeof(void *));
    else
        tmp = realloc(*v, sizeof(void *) * (nmemb + 2));

    if(!tmp)
        return -1;
    
    tmp[nmemb + 0] = p;
    tmp[nmemb + 1] = NULL;

    *v = tmp;

    return nmemb + 1;
}

static int dellv(char ***v, char *p) {
    int move = 0;
    char **pos, **n;
    
    for(pos = *v, n = pos + 1; *pos != NULL; pos = n, n = pos + 1) {
        if(*pos == p) {
            move = 1;
        }

        if(move)
            *pos = *n;
    }

    if(*v == NULL)
        free(*v);

    return move ? 0 : -1;
}

int main() {
    int ret, count = 0;
    char **p, **v = NULL;


    for(int i = 0; arr[i] != NULL; i++) {
        printf("%p: %s\n", arr[i], arr[i]);
        ret = addv(&v, arr[i], count);
        if(ret < 0)
            return -1;

        count = ret;
    }
    
    if(!dellv(&v, arr[1]))
        count--;
    
    if(!dellv(&v, arr[2]))
        count--;

    printf("v: %i\n", count);
    for(p = v; *p != NULL; p++) {
        printf("%p: %s\n", *p, *p);
    }
    
    if(!dellv(&v, arr[0]))
        count--;

    return 0;
}