#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdint.h>
#include <pthread.h>

/** struct queue_s - the queue handle
 * @lock: queue mutex lock
 * @cond_write: wait until data can be written
 * @cond_read: wait until data is written
 * @cond_empty: wait until the queue is empty
 * @statically_allocated: buffer was allocated statically
 * @nmemb: space for n elements
 * @size: size of a single element
 * @avail: avilable messages
 * @p_head: start of buffer
 * @p_tail: end of buffer
 * @p_write: current write pointer
 * @p_read: current read pointer
 */

struct queue_s {
    pthread_mutex_t lock;
    pthread_cond_t cond_write;
    pthread_cond_t cond_read;
    pthread_cond_t cond_empty;
    int statically_allocated;
    size_t nmemb;
    size_t size;
    size_t avail;
    void *p_head;
    void *p_tail;
    void *p_write;
    void *p_read;
};

struct queue_s *queue_create_static(void *p, size_t count, size_t nmemb, size_t size);
struct queue_s *queue_create(size_t nmemb, size_t size);
void queue_destroy(struct queue_s *q);
int queue_is_empty(struct queue_s *q);
int queue_is_full(struct queue_s *q);
int queue_space_available(struct queue_s *q);
int queue_send(struct queue_s *q, void *p, int timeout);
int queue_recv(struct queue_s *q, void *p, int timeout);
int queue_empty_wait(struct queue_s *q, int timeout);

#endif /* OHMYLIB_QUEUE_H_ */