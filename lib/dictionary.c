#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "dictionary.h"

#define ALIGN(x, mask) (((x) + (mask)) & ~(mask))

/** Minimal allocated number of entries in a dictionary */
#define DICTIONARY_MIN_SIZE (128u)

static int dictionary_find_idx_by_key(const struct dictionary_s *d, const char *key, unsigned hash, size_t *idx) {
    for(size_t i = 0; i < d->size; i++) {
        struct dictionary_entry_s *e = &d->entries[i];
        if(e->key == NULL) {
            continue;
        }

        if(hash != e->hash) {
            continue;
        }

        if(!strcmp(key, e->key)) {
            if(idx) {
                *idx = i;
            }
            return 0;
        }
    }

    errno = ENOENT;
    return -1;
}

static int dictionary_find_idx_by_empty(const struct dictionary_s *d, size_t *idx) {
    for(size_t i = 0; i < d->size; i++) {
        struct dictionary_entry_s *e = &d->entries[i];
        if(e->key == NULL) {
            if(idx) {
                *idx = i;
            }
            return 0;
        }
    }

    errno = ENOENT;
    return -1;
}

static int dictionary_resize(struct dictionary_s *d) {
    struct dictionary_entry_s *new_entries;
    size_t inc = DICTIONARY_MIN_SIZE;

    if(d->count < d->size) {
        return 0;
    }

    new_entries = calloc(d->size + inc, sizeof(*new_entries));
    if(!new_entries) {
        return -1;
    }

    (void)memcpy(new_entries, d->entries, d->size * sizeof(*new_entries));
    free(d->entries);
    d->size += inc;
    d->entries = new_entries;
    return 0;
}

unsigned dictionary_hash(const char *key) {
    size_t len, i;
    unsigned hash;

    if(!key) {
        return 0;
    }

    len = strlen(key);
    for(hash = 0, i = 0; i < len; i++) {
        hash += (unsigned)key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

struct dictionary_s *dictionary_create(size_t size) {
    struct dictionary_s *d;

    if(size < DICTIONARY_MIN_SIZE) {
        size = DICTIONARY_MIN_SIZE;
    }

    size = ALIGN(size, 0x7fu);
    d = (struct dictionary_s *)calloc(1, sizeof(*d));
    if(!d) {
        goto err;
    }

    d->size = size;
    d->entries = (struct dictionary_entry_s *)calloc(size, sizeof(*d->entries));
    if(!d->entries) {
        goto err_free;
    }

    return d;

err_free:
    free(d);
err:
    return NULL;
}

void dictionary_destroy(struct dictionary_s *d) {
    if(d == NULL) {
        return;
    }

    for(size_t i = 0; i < d->size; i++) {
        struct dictionary_entry_s *e = &d->entries[i];

        if(e->key) {
            free(e->key);
        }

        if(e->val) {
            free(e->val);
        }
    }

    free(d);
    return;
}

const char *dictionary_get(const struct dictionary_s *d, const char *key, const char *def) {
    struct dictionary_entry_s *e = NULL;
    unsigned hash;
    size_t idx;

    if(d == NULL || key == NULL) {
        errno = EINVAL;
        return def;
    }

    hash = dictionary_hash(key);
    if(dictionary_find_idx_by_key(d, key, hash, &idx) < 0) {
        return def;
    }

    e = &d->entries[idx];
    return e->val;
}

int dictionary_add(struct dictionary_s *d, const char *key, const char *val) {
    struct dictionary_entry_s *e = NULL;
    unsigned hash;
    size_t idx;

    if(d == NULL || key == NULL) {
        errno = EINVAL;
        return -1;
    }

    hash = dictionary_hash(key);
    if(dictionary_find_idx_by_key(d, key, hash, &idx) == 0) {
        e = &d->entries[idx];
        /** refresh value for existing key */
        if(e->val) {
            free(e->val);
        }
        e->val = strdup(val);
        if(!e->val) {
            return -1;
        }
        return 0;
    }

    if(dictionary_resize(d) < 0) {
        return -1;
    }

    if(dictionary_find_idx_by_empty(d, &idx) == 0) {
        e = &d->entries[idx];
        e->hash = hash;
        e->key = strdup(key);
        e->val = (val == NULL) ? NULL : strdup(val);
        d->count++;
        return 0;
    }

    return -1;
}

void dictionary_del(struct dictionary_s *d, const char *key) {
    struct dictionary_entry_s *e = NULL;
    unsigned hash;
    size_t idx;

    if(d == NULL || key == NULL) {
        errno = EINVAL;
        return;
    }

    hash = dictionary_hash(key);
    if(dictionary_find_idx_by_key(d, key, hash, &idx) < 0) {
        return;
    }

    e = &d->entries[idx];
    e->hash = 0;
    free(e->key);
    e->key = NULL;
    if(e->val) {
        free(e->val);
        e->val = NULL;
    }
    d->count--;
    return;
}

void dictionary_dump(const struct dictionary_s *d, FILE *out) {
    size_t  i;

    if(d == NULL || out == NULL) {
        return;
    }

    if(d->count == 0) {
        return;
    }

    for(i = 0; i < d->size; i++) {
        struct dictionary_entry_s *e = &d->entries[i];
        if(e->key == NULL) {
            continue;
        }
        fprintf(out, "%08X;\"%s\";\"%s\"\n",
                e->hash,
                e->key,
                e->val ? e->val : "undefined");
    }

    return;
}