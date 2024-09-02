#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <cstring.h>

char *substr(char *haystack, char **nxt, char needle) {
    char *n = NULL;
    size_t len = 0;

    if(!haystack) {
        errno = EINVAL;
        return NULL;
    }

    n = strchr(haystack, needle);
    if(n == NULL) {
        len = strlen(haystack);
    }
    else {
        len = (n - haystack);
    }

    if(nxt) {
        *nxt = (n != NULL) ? n + 1 : n;
    }

    return strndup(haystack, len);
}