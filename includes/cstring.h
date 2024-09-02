#ifndef CSTRING_H_
#define CSTRING_H_

#include <stddef.h>

struct cstring_s {
    size_t size;
    char p[];
};

#define iter_substr(str, h, n, s) for(str = substr(h, &n, s); str != NULL; str = substr(n, &n, s))

char *substr(char *haystack, char **nxt, char needle);

#endif