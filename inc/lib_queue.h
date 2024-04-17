#ifndef OHMYLIB_QUEUE_H_
#define OHMYLIB_QUEUE_H_

#include <stdint.h>
#include <pthread.h>

struct lib_queue_s {
    pthread_mutex_t lock;
    pthread_cond_t cond_write;
    pthread_cond_t cond_read;
    pthread_cond_t cond_empty;
    int statically_allocated;
    uint32_t nmemb; /**< element count */
    uint32_t size; /**< element size */
    uint32_t avail; /**< messages available */
    void *p_head;
    void *p_tail;
    void *p_write;
    void *p_read;
};

struct lib_queue_s *lib_queue_create_static(void *p, uint32_t count, uint32_t nmemb, uint32_t size);
struct lib_queue_s *lib_queue_create(uint32_t nmemb, uint32_t size);
void lib_queue_destroy(struct lib_queue_s *q);
int lib_queue_is_empty(struct lib_queue_s *q);
int lib_queue_is_full(struct lib_queue_s *q);
int lib_queue_space_available(struct lib_queue_s *q);
int lib_queue_send(struct lib_queue_s *q, void *p, int timeout);
int lib_queue_recv(struct lib_queue_s *q, void *p, int timeout);
int lib_queue_empty_wait(struct lib_queue_s *q, int timeout);

#endif /* OHMYLIB_QUEUE_H_ */