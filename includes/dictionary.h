#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <stdio.h>

/** struct dictionary_entry_s - a dictionary entry
 * @hash: hash value for key
 * @key: string key
 * @val: string value
 */
struct dictionary_entry_s {
    unsigned hash;
    char *key;
    char *val;
};

/** struct dictionary_s - 
 * @count: number of entries in dictionary
 * @size: size of storage in dictionary
 * @entries: list of entries
 */
struct dictionary_s {
    size_t count;
    size_t size;
    struct dictionary_entry_s *entries;
};

struct dictionary_s *dictionary_create(size_t size);
void dictionary_destroy(struct dictionary_s *d);

unsigned dictionary_hash(const char *key);
const char *dictionary_get(const struct dictionary_s *d, const char *key, const char *def);
int dictionary_add(struct dictionary_s *d, const char *key, const char *val);
void dictionary_del(struct dictionary_s *d, const char *key);
void dictionary_dump(const struct dictionary_s *d, FILE *out);

#endif