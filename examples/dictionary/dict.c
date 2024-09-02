#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <errno.h>
#include <string.h>

#include <dictionary.h>

static void dictionary_store(const char *fn, struct dictionary_s *d) {
    FILE *f;

    f = fopen(fn, "w+");
    if(!f) {
        fprintf(stderr, "failed to open %s: %s\n", fn, strerror(errno));
        return;
    }

    dictionary_dump(d, f);
    fclose(f);
}

int main() {
    struct dictionary_s *dict;
    char key[128], val[1024];

    dict = dictionary_create(0);
    if(!dict) {
        fprintf(stderr, "failed to create dict: %s\n", strerror(errno));
    }

    for(int i = 0; i < 4096; i++) {
        snprintf(key, sizeof(key), "my_key;%d", i);
        snprintf(val, sizeof(val), "my_val%d \"%d\"", i, i * i);
        dictionary_add(dict, key, val);
    }

    dictionary_store("dict.txt", dict);

    dictionary_destroy(dict);
    return 0;
}