#ifndef LIB_RINGBUFFER_H_
#define LIB_RINGBUFFER_H_

#include <stdint.h>
#include <pthread.h>
#include <list.h>

struct lib_rb_descriptor_s {
    struct list_head entry;
    struct lib_rb_s *rb;
    void *p;
};

struct lib_rb_s {
    int static_allocation;
    size_t desc_free_count;
    size_t watermark;
    pthread_mutex_t lock;
    struct list_head desc_free;
    struct list_head desc_locked;
    struct lib_rb_descriptor_s *desc;
    void *p; /**< data pool */
};

struct lib_rb_descriptor_s *lib_rb_get(struct lib_rb_s *rb);
void lib_rb_put(struct lib_rb_descriptor_s *desc);

int lib_rb_create_static(struct lib_rb_s *rb, size_t size, size_t nmemb, void *p, size_t count);
int lib_rb_create(struct lib_rb_s *rb, size_t size, size_t nmemb);
void lib_rb_destroy(struct lib_rb_s *rb);

#endif